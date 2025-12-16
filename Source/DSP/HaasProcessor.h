#pragma once

#include "DelayLine.h"
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
    bool phaseInvertLeft = false;
    bool phaseInvertRight = false;
    bool bypass = false;
};

/**
 * @brief Core Haas effect and alignment processor
 *
 * This class handles stereo delay, width adjustment (mid/side),
 * phase inversion, and correlation metering. Designed to be
 * independent of any audio framework for testability.
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

        // Initialize smoothing (20ms ramp time)
        smoothingCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.02f));

        reset();
    }

    /**
     * @brief Reset the processor state
     */
    void reset()
    {
        delayLineL.clear();
        delayLineR.clear();

        smoothedDelayL = 0.0f;
        smoothedDelayR = 0.0f;
        smoothedWidth = 100.0f;
        smoothedMix = 100.0f;

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
    }

    /**
     * @brief Set the processing parameters
     * @param newParams New parameters to apply (smoothed over time)
     */
    void setParameters(const HaasParameters& newParams)
    {
        params = newParams;
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

        // Smooth parameter changes
        smoothedDelayL += (params.delayLeftMs - smoothedDelayL) * smoothingCoeff;
        smoothedDelayR += (params.delayRightMs - smoothedDelayR) * smoothingCoeff;
        smoothedWidth += (params.width - smoothedWidth) * smoothingCoeff;
        smoothedMix += (params.mix - smoothedMix) * smoothingCoeff;

        // Process through delay lines
        float delayedL = delayLineL.process(input.left, smoothedDelayL);
        float delayedR = delayLineR.process(input.right, smoothedDelayR);

        // Apply phase inversion
        if (params.phaseInvertLeft)
            delayedL = -delayedL;
        if (params.phaseInvertRight)
            delayedR = -delayedR;

        // Apply width (mid/side processing)
        float mid = (delayedL + delayedR) * 0.5f;
        float side = (delayedL - delayedR) * 0.5f;
        float widthFactor = smoothedWidth / 100.0f;
        side *= widthFactor;

        float processedL = mid + side;
        float processedR = mid - side;

        // Apply dry/wet mix
        float mixFactor = smoothedMix / 100.0f;
        StereoSample output;
        output.left = input.left * (1.0f - mixFactor) + processedL * mixFactor;
        output.right = input.right * (1.0f - mixFactor) + processedR * mixFactor;

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
     * @return Metering data (levels and correlation)
     */
    MeteringData getMetering() const { return metering; }

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

        // Reset accumulators
        peakInputLevel = 0.0f;
        peakOutputLevel = 0.0f;
        correlationSum = 0.0f;
        leftSquaredSum = 0.0f;
        rightSquaredSum = 0.0f;
        meteringBlockCount = 0;
    }

    // Delay lines
    DelayLine delayLineL;
    DelayLine delayLineR;

    // Parameters
    HaasParameters params;
    double currentSampleRate = 44100.0;

    // Smoothed parameter values
    float smoothedDelayL = 0.0f;
    float smoothedDelayR = 0.0f;
    float smoothedWidth = 100.0f;
    float smoothedMix = 100.0f;
    float smoothingCoeff = 0.001f;

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
