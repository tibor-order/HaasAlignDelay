#pragma once

#include "DelayLine.h"
#include "AutoPhaseCorrector.h"
#include <cmath>
#include <algorithm>

namespace DSP
{

/**
 * @brief Stereo sample pair for processing
 */
struct StereoSample
{
    float left = 0.0f;
    float right = 0.0f;
};

/**
 * @brief Metering data from the processor
 */
struct MeteringData
{
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    float correlation = 1.0f;
    float weightedCorrelation = 1.0f;
    float correctionAmount = 0.0f;
    float effectiveWidth = 100.0f;
    bool correctionActive = false;
    bool transientDetected = false;
};

/**
 * @brief Parameters for the Haas processor
 */
struct HaasParameters
{
    float delayLeftMs = 0.0f;
    float delayRightMs = 0.0f;
    float width = 100.0f;       // 0-200%, 100% = no change
    float mix = 100.0f;         // 0-100%
    float outputGain = 0.0f;    // -12 to +12 dB, applied after mix
    float widthLowCut = 250.0f; // 20-500Hz, high-pass on side channel
    float correctionSpeed = 50.0f; // 0-100%, affects attack/release of auto phase
    bool phaseInvertLeft = false;
    bool phaseInvertRight = false;
    bool bypass = false;            // Master bypass
    bool autoPhaseEnabled = false;
    bool delayLink = false;         // When true, L/R delays adjust together maintaining offset
    bool delayBypass = false;       // When true, delay processing is bypassed
    bool widthBypass = false;       // When true, width processing is bypassed
    bool phaseBypass = false;       // When true, auto phase correction is bypassed
    bool outputBypass = false;      // When true, output gain is bypassed (unity)
    PhaseSafetyMode phaseSafety = PhaseSafetyMode::Balanced;
};

/**
 * @brief Core Haas effect and alignment processor
 *
 * This class handles stereo delay, width adjustment (mid/side),
 * phase inversion, correlation metering, and intelligent auto phase correction.
 * Designed to be independent of any audio framework for testability.
 */
class HaasProcessor
{
public:
    static constexpr float MAX_DELAY_MS = 50.0f;

    HaasProcessor() = default;
    ~HaasProcessor() = default;

    /**
     * @brief Prepare the processor for playback
     * @param sampleRate Sample rate in Hz
     * @param maxBlockSize Maximum expected block size
     */
    void prepare(double sampleRate, int maxBlockSize)
    {
        currentSampleRate = sampleRate;

        // Prepare delay lines with extra headroom
        float maxDelayWithHeadroom = MAX_DELAY_MS + (maxBlockSize / static_cast<float>(sampleRate) * 1000.0f);
        delayLineL.prepare(sampleRate, maxDelayWithHeadroom);
        delayLineR.prepare(sampleRate, maxDelayWithHeadroom);

        // Prepare low-band delay lines (for 2-band processing)
        delayLineLowL.prepare(sampleRate, maxDelayWithHeadroom);
        delayLineLowR.prepare(sampleRate, maxDelayWithHeadroom);

        // Prepare auto phase corrector
        autoPhaseCorrector.prepare(sampleRate, maxBlockSize);

        // Prepare crossover for 2-band processing
        crossover.prepare(sampleRate, AutoPhaseCorrector::CROSSOVER_HZ);

        // Initialize smoothing (20ms ramp time for general params)
        smoothingCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.02f));

        // Initialize output gain smoothing (50ms ramp time for click-free gain changes)
        outputGainSmoothingCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.05f));

        // Initialize filter smoothing (100ms for smooth frequency transitions)
        filterSmoothingCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.1f));

        // Initialize width low-cut filter at default frequency
        widthLowCutFilter.setHighPass(sampleRate, 250.0);

        reset();
    }

    /**
     * @brief Reset the processor state
     */
    void reset()
    {
        delayLineL.clear();
        delayLineR.clear();
        delayLineLowL.clear();
        delayLineLowR.clear();

        crossover.reset();
        autoPhaseCorrector.reset();

        smoothedDelayL = 0.0f;
        smoothedDelayR = 0.0f;
        smoothedWidth = 100.0f;
        smoothedMix = 100.0f;
        smoothedOutputGain = 1.0f;  // Unity gain (0dB)
        smoothedWidthLowCut = 250.0f;

        widthLowCutFilter.reset();

        // Effective (corrected) parameters
        effectiveDelayL = 0.0f;
        effectiveDelayR = 0.0f;
        effectiveWidth = 100.0f;

        correlationSum = 0.0f;
        leftSquaredSum = 0.0f;
        rightSquaredSum = 0.0f;
        meteringBlockCount = 0;
    }

    /**
     * @brief Release resources
     */
    void release()
    {
        delayLineL.release();
        delayLineR.release();
        delayLineLowL.release();
        delayLineLowR.release();
    }

    /**
     * @brief Set the processing parameters
     * @param newParams New parameters to apply (smoothed over time)
     */
    void setParameters(const HaasParameters& newParams)
    {
        params = newParams;

        // Update auto phase corrector state
        // Enable only if autoPhase is on AND phase is not bypassed
        autoPhaseCorrector.setEnabled(params.autoPhaseEnabled && !params.phaseBypass);
        autoPhaseCorrector.setSafetyMode(params.phaseSafety);
        autoPhaseCorrector.setCorrectionSpeed(params.correctionSpeed);
    }

    /**
     * @brief Get the current parameters
     * @return Current parameters
     */
    const HaasParameters& getParameters() const { return params; }

    /**
     * @brief Process a single stereo sample
     * @param input Input stereo sample
     * @return Processed stereo sample
     */
    StereoSample processSample(StereoSample input)
    {
        // Update metering for input
        updateInputMetering(input);

        if (params.bypass)
        {
            updateOutputMetering(input);
            return input;
        }

        // Smooth user parameter changes
        smoothedDelayL += (params.delayLeftMs - smoothedDelayL) * smoothingCoeff;
        smoothedDelayR += (params.delayRightMs - smoothedDelayR) * smoothingCoeff;
        smoothedWidth += (params.width - smoothedWidth) * smoothingCoeff;
        smoothedMix += (params.mix - smoothedMix) * smoothingCoeff;

        // Update width low-cut filter frequency (smoothed)
        float prevLowCut = smoothedWidthLowCut;
        smoothedWidthLowCut += (params.widthLowCut - smoothedWidthLowCut) * filterSmoothingCoeff;
        if (std::abs(smoothedWidthLowCut - prevLowCut) > 0.1f)
        {
            widthLowCutFilter.setHighPass(currentSampleRate, smoothedWidthLowCut);
        }

        // Get corrected parameters from auto phase corrector
        float correctedDelayL, correctedDelayR, correctedWidth;
        autoPhaseCorrector.getCorrectedParameters(
            smoothedDelayL, smoothedDelayR, smoothedWidth,
            correctedDelayL, correctedDelayR, correctedWidth);

        // Smooth the corrected values for zipper-free transitions
        effectiveDelayL += (correctedDelayL - effectiveDelayL) * smoothingCoeff;
        effectiveDelayR += (correctedDelayR - effectiveDelayR) * smoothingCoeff;
        effectiveWidth += (correctedWidth - effectiveWidth) * smoothingCoeff;

        // If delay is bypassed, use zero delay
        float actualDelayL = params.delayBypass ? 0.0f : effectiveDelayL;
        float actualDelayR = params.delayBypass ? 0.0f : effectiveDelayR;

        // === 2-BAND PROCESSING ===
        // Split into low and high bands
        float lowL, lowR, highL, highR;
        crossover.process(input.left, input.right, lowL, lowR, highL, highR);

        // --- Process LOW BAND (conservative) ---
        float bassDelayL, bassDelayR, bassWidth;
        autoPhaseCorrector.getBassLimitedParameters(
            actualDelayL, actualDelayR, effectiveWidth,
            bassDelayL, bassDelayR, bassWidth);

        float delayedLowL = delayLineLowL.process(lowL, bassDelayL);
        float delayedLowR = delayLineLowR.process(lowR, bassDelayR);

        // Apply phase inversion to low band
        if (params.phaseInvertLeft) delayedLowL = -delayedLowL;
        if (params.phaseInvertRight) delayedLowR = -delayedLowR;

        // Apply width to low band (limited) - if width is not bypassed
        float lowMid = (delayedLowL + delayedLowR) * 0.5f;
        float lowSide = (delayedLowL - delayedLowR) * 0.5f;
        if (!params.widthBypass)
        {
            lowSide *= bassWidth / 100.0f;
        }
        float processedLowL = lowMid + lowSide;
        float processedLowR = lowMid - lowSide;

        // --- Process HIGH BAND (full user control with correction) ---
        float delayedHighL = delayLineL.process(highL, actualDelayL);
        float delayedHighR = delayLineR.process(highR, actualDelayR);

        // Apply phase inversion to high band
        if (params.phaseInvertLeft) delayedHighL = -delayedHighL;
        if (params.phaseInvertRight) delayedHighR = -delayedHighR;

        // Apply width to high band (corrected) - if width is not bypassed
        float highMid = (delayedHighL + delayedHighR) * 0.5f;
        float highSide = (delayedHighL - delayedHighR) * 0.5f;
        if (!params.widthBypass)
        {
            highSide *= effectiveWidth / 100.0f;
        }
        float processedHighL = highMid + highSide;
        float processedHighR = highMid - highSide;

        // --- Combine bands ---
        float processedL = processedLowL + processedHighL;
        float processedR = processedLowR + processedHighR;

        // Apply width low-cut filter to combined signal (if width is not bypassed)
        // This applies HP filter to side channel to keep bass mono
        if (!params.widthBypass)
        {
            float combinedMid = (processedL + processedR) * 0.5f;
            float combinedSide = (processedL - processedR) * 0.5f;
            // Filter the side channel - bass frequencies pass through mid only
            combinedSide = widthLowCutFilter.process(combinedSide);
            processedL = combinedMid + combinedSide;
            processedR = combinedMid - combinedSide;
        }

        // === PSYCHOACOUSTIC COMPENSATION ===
        // When correction is active, add subtle mid boost to maintain perceived width
        float compensation = autoPhaseCorrector.getPsychoacousticCompensation();
        if (compensation > 0.001f)
        {
            // Apply subtle mid-boost (approximately +1-2dB @ 1-2kHz)
            float midBoost = 1.0f + compensation;
            float mid = (processedL + processedR) * 0.5f * midBoost;
            float side = (processedL - processedR) * 0.5f;
            processedL = mid + side;
            processedR = mid - side;
        }

        // Apply dry/wet mix
        float mixFactor = smoothedMix / 100.0f;
        StereoSample output;
        output.left = input.left * (1.0f - mixFactor) + processedL * mixFactor;
        output.right = input.right * (1.0f - mixFactor) + processedR * mixFactor;

        // Apply output gain (if output module is not bypassed)
        if (!params.outputBypass)
        {
            // Convert dB to linear and smooth
            float targetGain = std::pow(10.0f, params.outputGain / 20.0f);
            smoothedOutputGain += (targetGain - smoothedOutputGain) * outputGainSmoothingCoeff;

            output.left *= smoothedOutputGain;
            output.right *= smoothedOutputGain;
        }

        // Feed output to auto phase corrector for analysis
        autoPhaseCorrector.processSample(output.left, output.right);

        // Update metering for output
        updateOutputMetering(output);

        return output;
    }

    /**
     * @brief Process a block of stereo audio
     * @param leftChannel Pointer to left channel buffer (modified in place)
     * @param rightChannel Pointer to right channel buffer (modified in place)
     * @param numSamples Number of samples to process
     */
    void processBlock(float* leftChannel, float* rightChannel, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            StereoSample input{leftChannel[i], rightChannel[i]};
            StereoSample output = processSample(input);
            leftChannel[i] = output.left;
            rightChannel[i] = output.right;
        }

        // Finalize metering at end of block
        finalizeMetering();
    }

    /**
     * @brief Get current metering data
     * @return Metering data (levels, correlation, correction state)
     */
    MeteringData getMetering() const { return metering; }

    /**
     * @brief Get the auto phase corrector for direct access to extended metering
     */
    const AutoPhaseCorrector& getAutoPhaseCorrector() const { return autoPhaseCorrector; }

    /**
     * @brief Convert milliseconds to samples
     * @param ms Time in milliseconds
     * @return Number of samples
     */
    float msToSamples(float ms) const
    {
        return ms * 0.001f * static_cast<float>(currentSampleRate);
    }

    /**
     * @brief Get the current sample rate
     * @return Sample rate in Hz
     */
    double getSampleRate() const { return currentSampleRate; }

private:
    void updateInputMetering(StereoSample sample)
    {
        float level = std::max(std::abs(sample.left), std::abs(sample.right));
        peakInputLevel = std::max(peakInputLevel, level);
    }

    void updateOutputMetering(StereoSample sample)
    {
        float level = std::max(std::abs(sample.left), std::abs(sample.right));
        peakOutputLevel = std::max(peakOutputLevel, level);

        // Correlation accumulation
        correlationSum += sample.left * sample.right;
        leftSquaredSum += sample.left * sample.left;
        rightSquaredSum += sample.right * sample.right;
        meteringBlockCount++;
    }

    void finalizeMetering()
    {
        metering.inputLevel = peakInputLevel;
        metering.outputLevel = peakOutputLevel;

        // Calculate correlation coefficient
        float denom = std::sqrt(leftSquaredSum * rightSquaredSum);
        if (denom > 0.0001f)
            metering.correlation = correlationSum / denom;
        else
            metering.correlation = 1.0f;

        // Get auto phase metering
        const auto& autoMetering = autoPhaseCorrector.getMetering();
        metering.weightedCorrelation = autoMetering.weightedCorrelation;
        metering.correctionAmount = autoMetering.correctionAmount;
        metering.effectiveWidth = autoMetering.effectiveWidth;
        metering.correctionActive = autoMetering.correctionActive;
        metering.transientDetected = autoMetering.transientDetected;

        // Reset accumulators
        peakInputLevel = 0.0f;
        peakOutputLevel = 0.0f;
        correlationSum = 0.0f;
        leftSquaredSum = 0.0f;
        rightSquaredSum = 0.0f;
        meteringBlockCount = 0;
    }

    // Delay lines - high band
    DelayLine delayLineL;
    DelayLine delayLineR;

    // Delay lines - low band (conservative)
    DelayLine delayLineLowL;
    DelayLine delayLineLowR;

    // Crossover for 2-band processing
    LinkwitzRileyCrossover crossover;

    // Width low-cut high-pass filter (applied to side channel)
    BiquadFilter widthLowCutFilter;

    // Auto phase corrector
    AutoPhaseCorrector autoPhaseCorrector;

    // Parameters
    HaasParameters params;
    double currentSampleRate = 44100.0;

    // Smoothed user parameter values
    float smoothedDelayL = 0.0f;
    float smoothedDelayR = 0.0f;
    float smoothedWidth = 100.0f;
    float smoothedMix = 100.0f;
    float smoothedOutputGain = 1.0f;  // Linear gain (1.0 = 0dB)
    float smoothedWidthLowCut = 250.0f;  // Smoothed low cut frequency
    float smoothingCoeff = 0.001f;
    float outputGainSmoothingCoeff = 0.001f;  // Separate smoother for output gain (50ms)
    float filterSmoothingCoeff = 0.001f;  // Slower for filter frequency changes

    // Effective (corrected) parameter values
    float effectiveDelayL = 0.0f;
    float effectiveDelayR = 0.0f;
    float effectiveWidth = 100.0f;

    // Metering
    MeteringData metering;
    float peakInputLevel = 0.0f;
    float peakOutputLevel = 0.0f;
    float correlationSum = 0.0f;
    float leftSquaredSum = 0.0f;
    float rightSquaredSum = 0.0f;
    int meteringBlockCount = 0;
};

} // namespace DSP
