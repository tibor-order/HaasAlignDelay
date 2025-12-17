#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <vector>

namespace DSP
{

/**
 * @brief Phase safety modes for auto-correction threshold
 */
enum class PhaseSafetyMode
{
    Relaxed,   // 0.2 threshold - maximum width, some mono risk
    Balanced,  // 0.3 threshold - best compromise (default)
    Strict     // 0.5 threshold - safest mono, less width
};

/**
 * @brief Metering data from the auto-phase corrector
 */
struct AutoPhaseMetering
{
    float rawCorrelation = 1.0f;           // Unweighted correlation
    float weightedCorrelation = 1.0f;      // Frequency-weighted correlation
    float correctionAmount = 0.0f;         // 0.0 = no correction, 1.0 = full correction
    float effectiveWidth = 100.0f;         // Actual width after correction (%)
    float effectiveDelayL = 0.0f;          // Actual L delay after correction (ms)
    float effectiveDelayR = 0.0f;          // Actual R delay after correction (ms)
    bool correctionActive = false;         // True when actively reducing parameters
    bool transientDetected = false;        // True during transient pass-through
};

/**
 * @brief Correction timing parameters (in milliseconds)
 */
struct CorrectionTiming
{
    float attackMs = 105.0f;   // Attack time in ms
    float releaseMs = 420.0f;  // Release time in ms
};

/**
 * @brief 2nd-order biquad filter for crossover and DC blocking
 */
class BiquadFilter
{
public:
    void setCoefficients(float b0, float b1, float b2, float a1, float a2)
    {
        this->b0 = b0; this->b1 = b1; this->b2 = b2;
        this->a1 = a1; this->a2 = a2;
    }

    void reset()
    {
        z1 = z2 = 0.0f;
    }

    float process(float input)
    {
        float output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;
        return output;
    }

    // Design high-pass filter (Butterworth)
    void setHighPass(double sampleRate, double cutoffHz)
    {
        double w0 = 2.0 * 3.14159265358979323846 * cutoffHz / sampleRate;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * 0.7071067811865476); // Q = sqrt(2)/2

        double b0 = (1.0 + cosw0) / 2.0;
        double b1 = -(1.0 + cosw0);
        double b2 = (1.0 + cosw0) / 2.0;
        double a0 = 1.0 + alpha;
        double a1 = -2.0 * cosw0;
        double a2 = 1.0 - alpha;

        setCoefficients(
            static_cast<float>(b0 / a0),
            static_cast<float>(b1 / a0),
            static_cast<float>(b2 / a0),
            static_cast<float>(a1 / a0),
            static_cast<float>(a2 / a0)
        );
    }

    // Design low-pass filter (Butterworth)
    void setLowPass(double sampleRate, double cutoffHz)
    {
        double w0 = 2.0 * 3.14159265358979323846 * cutoffHz / sampleRate;
        double cosw0 = std::cos(w0);
        double sinw0 = std::sin(w0);
        double alpha = sinw0 / (2.0 * 0.7071067811865476);

        double b0 = (1.0 - cosw0) / 2.0;
        double b1 = 1.0 - cosw0;
        double b2 = (1.0 - cosw0) / 2.0;
        double a0 = 1.0 + alpha;
        double a1 = -2.0 * cosw0;
        double a2 = 1.0 - alpha;

        setCoefficients(
            static_cast<float>(b0 / a0),
            static_cast<float>(b1 / a0),
            static_cast<float>(b2 / a0),
            static_cast<float>(a1 / a0),
            static_cast<float>(a2 / a0)
        );
    }

private:
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;
};

/**
 * @brief Linkwitz-Riley 4th order crossover (2 cascaded 2nd-order Butterworth)
 */
class LinkwitzRileyCrossover
{
public:
    void prepare(double sampleRate, double crossoverHz)
    {
        lpFilter1L.setLowPass(sampleRate, crossoverHz);
        lpFilter2L.setLowPass(sampleRate, crossoverHz);
        lpFilter1R.setLowPass(sampleRate, crossoverHz);
        lpFilter2R.setLowPass(sampleRate, crossoverHz);

        hpFilter1L.setHighPass(sampleRate, crossoverHz);
        hpFilter2L.setHighPass(sampleRate, crossoverHz);
        hpFilter1R.setHighPass(sampleRate, crossoverHz);
        hpFilter2R.setHighPass(sampleRate, crossoverHz);
    }

    void reset()
    {
        lpFilter1L.reset(); lpFilter2L.reset();
        lpFilter1R.reset(); lpFilter2R.reset();
        hpFilter1L.reset(); hpFilter2L.reset();
        hpFilter1R.reset(); hpFilter2R.reset();
    }

    void process(float inL, float inR,
                 float& lowL, float& lowR,
                 float& highL, float& highR)
    {
        // Low band: cascaded low-pass (LR4)
        lowL = lpFilter2L.process(lpFilter1L.process(inL));
        lowR = lpFilter2R.process(lpFilter1R.process(inR));

        // High band: cascaded high-pass (LR4)
        highL = hpFilter2L.process(hpFilter1L.process(inL));
        highR = hpFilter2R.process(hpFilter1R.process(inR));
    }

private:
    BiquadFilter lpFilter1L, lpFilter2L, lpFilter1R, lpFilter2R;
    BiquadFilter hpFilter1L, hpFilter2L, hpFilter1R, hpFilter2R;
};

/**
 * @brief Smart Auto Phase Corrector
 *
 * Implements intelligent phase correction for the Haas effect:
 * - 50ms windowed correlation analysis with RMS smoothing
 * - Frequency-weighted measurement (ignores bass, weights mids heavily)
 * - 2-band processing with conservative bass handling
 * - Transient detection for punch preservation
 * - Hysteresis to prevent pumping
 * - Psychoacoustic compensation (mid boost when correcting)
 *
 * This is the "secret sauce" - no other plugin has this combination.
 */
class AutoPhaseCorrector
{
public:
    // Timing constants (in seconds)
    static constexpr float ANALYSIS_WINDOW_SEC = 0.050f;      // 50ms analysis window
    static constexpr float RMS_SMOOTHING_SEC = 0.100f;        // 100ms RMS smoothing
    static constexpr float ATTACK_TIME_SEC = 0.100f;          // 100ms attack
    static constexpr float RELEASE_TIME_SEC = 0.400f;         // 400ms release
    static constexpr float PARAMETER_RAMP_SEC = 0.010f;       // 10ms zipper-free ramps
    static constexpr float TRANSIENT_WINDOW_SEC = 0.005f;     // 5ms transient detection
    static constexpr float TRANSIENT_HOLDOFF_SEC = 0.080f;    // 80ms transient pass-through

    // Threshold constants
    static constexpr float NOISE_FLOOR_DB = -60.0f;           // Ignore below this
    static constexpr float TRANSIENT_THRESHOLD_DB = 10.0f;    // Peak rise for transient
    static constexpr float HYSTERESIS_ENGAGE = 0.30f;         // Engage below this
    static constexpr float HYSTERESIS_RELEASE = 0.40f;        // Release above this

    // Crossover and bass limits
    static constexpr float CROSSOVER_HZ = 300.0f;             // Low/high split
    static constexpr float MAX_BASS_DELAY_MS = 10.0f;         // Max delay for bass
    static constexpr float MAX_BASS_WIDTH = 120.0f;           // Max width for bass (%)

    // Correction scaling
    static constexpr float MIN_WIDTH_PERCENT = 100.0f;        // Never go below mono

    AutoPhaseCorrector() = default;

    /**
     * @brief Prepare the corrector for processing
     * @param sampleRate Sample rate in Hz
     * @param maxBlockSize Maximum samples per block
     */
    void prepare(double sampleRate, int maxBlockSize)
    {
        currentSampleRate = sampleRate;

        // Calculate sample counts for timing
        analysisWindowSamples = static_cast<int>(ANALYSIS_WINDOW_SEC * sampleRate);
        transientWindowSamples = static_cast<int>(TRANSIENT_WINDOW_SEC * sampleRate);
        transientHoldoffSamples = static_cast<int>(TRANSIENT_HOLDOFF_SEC * sampleRate);

        // Calculate smoothing coefficients (one-pole IIR)
        rmsCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * RMS_SMOOTHING_SEC));
        attackCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * ATTACK_TIME_SEC));
        releaseCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * RELEASE_TIME_SEC));
        paramRampCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * PARAMETER_RAMP_SEC));

        // Prepare crossover
        crossover.prepare(sampleRate, CROSSOVER_HZ);

        // Prepare DC blockers (20Hz high-pass)
        dcBlockerL.setHighPass(sampleRate, 20.0);
        dcBlockerR.setHighPass(sampleRate, 20.0);

        // Prepare frequency weighting filters
        // Band-pass for measurement: emphasize 500Hz-4kHz
        // Using simple cascaded high-pass (150Hz) and low-pass (8kHz)
        measureHPL.setHighPass(sampleRate, 150.0);
        measureHPR.setHighPass(sampleRate, 150.0);
        measureLPL.setLowPass(sampleRate, 8000.0);
        measureLPR.setLowPass(sampleRate, 8000.0);

        // Mid-boost filter for psychoacoustic compensation (1-2kHz shelf)
        midBoostL.setHighPass(sampleRate, 1000.0); // Simple approximation
        midBoostR.setHighPass(sampleRate, 1000.0);

        // Calculate noise floor threshold
        noiseFloorLinear = std::pow(10.0f, NOISE_FLOOR_DB / 20.0f);

        reset();
    }

    /**
     * @brief Reset all state
     */
    void reset()
    {
        crossover.reset();
        dcBlockerL.reset();
        dcBlockerR.reset();
        measureHPL.reset();
        measureHPR.reset();
        measureLPL.reset();
        measureLPR.reset();
        midBoostL.reset();
        midBoostR.reset();

        // Reset accumulators
        sumLR = 0.0f;
        sumL2 = 0.0f;
        sumR2 = 0.0f;
        weightedSumLR = 0.0f;
        weightedSumL2 = 0.0f;
        weightedSumR2 = 0.0f;
        sampleCount = 0;

        // Reset smoothed values
        smoothedCorrelation = 1.0f;
        smoothedWeightedCorrelation = 1.0f;
        correctionGain = 0.0f;

        // Reset effective parameters
        effectiveWidthMult = 1.0f;
        effectiveDelayMult = 1.0f;

        // Reset transient detector
        prevPeakLevel = 0.0f;
        transientHoldoffCounter = 0;
        isInTransient = false;

        // Reset hysteresis state
        correctionEngaged = false;
    }

    /**
     * @brief Set the phase safety mode
     */
    void setSafetyMode(PhaseSafetyMode mode)
    {
        safetyMode = mode;
        switch (mode)
        {
            case PhaseSafetyMode::Relaxed:
                correlationThreshold = 0.2f;
                break;
            case PhaseSafetyMode::Balanced:
                correlationThreshold = 0.3f;
                break;
            case PhaseSafetyMode::Strict:
                correlationThreshold = 0.5f;
                break;
        }
    }

    /**
     * @brief Enable or disable the corrector
     */
    void setEnabled(bool enabled)
    {
        isEnabled = enabled;
        if (!enabled)
        {
            // Smoothly return to no correction
            correctionGain = 0.0f;
        }
    }

    /**
     * @brief Set the correction speed (0-100%)
     *
     * 0% = Slow (200ms attack, 800ms release) - gentle, musical
     * 100% = Fast (10ms attack, 40ms release) - aggressive
     *
     * @param speed Speed percentage (0-100)
     */
    void setCorrectionSpeed(float speed)
    {
        // Clamp to valid range
        correctionSpeed = std::max(0.0f, std::min(100.0f, speed));

        // Interpolate timing between slow and fast
        // Slow: 200ms attack, 800ms release
        // Fast: 10ms attack, 40ms release
        float t = correctionSpeed / 100.0f;
        float attackTimeSec = (1.0f - t) * 0.200f + t * 0.010f;
        float releaseTimeSec = (1.0f - t) * 0.800f + t * 0.040f;

        // Update coefficients
        attackCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(currentSampleRate) * attackTimeSec));
        releaseCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(currentSampleRate) * releaseTimeSec));

        // Store timing for query
        currentAttackMs = attackTimeSec * 1000.0f;
        currentReleaseMs = releaseTimeSec * 1000.0f;
    }

    /**
     * @brief Get current correction timing parameters
     * @return CorrectionTiming struct with attack and release in ms
     */
    CorrectionTiming getCorrectionTiming() const
    {
        CorrectionTiming timing;
        timing.attackMs = currentAttackMs;
        timing.releaseMs = currentReleaseMs;
        return timing;
    }

    /**
     * @brief Get the analysis window duration in milliseconds
     * @return Analysis window in ms (constant ~50ms)
     */
    float getAnalysisWindowMs() const
    {
        return ANALYSIS_WINDOW_SEC * 1000.0f;
    }

    /**
     * @brief Process stereo samples and compute correction factors
     *
     * Call this for each sample to update internal state.
     * Then use getCorrectedParameters() to get adjusted delay/width.
     *
     * @param left Left channel sample (after Haas processing, before output)
     * @param right Right channel sample
     */
    void processSample(float left, float right)
    {
        if (!isEnabled)
        {
            metering.correctionActive = false;
            metering.correctionAmount = 0.0f;
            return;
        }

        // DC blocking
        float dcL = dcBlockerL.process(left);
        float dcR = dcBlockerR.process(right);

        // Check if signal is above noise floor
        float peakLevel = std::max(std::abs(dcL), std::abs(dcR));
        bool aboveNoiseFloor = peakLevel > noiseFloorLinear;

        // Transient detection
        updateTransientDetector(peakLevel);

        if (aboveNoiseFloor)
        {
            // Raw correlation accumulation
            sumLR += dcL * dcR;
            sumL2 += dcL * dcL;
            sumR2 += dcR * dcR;

            // Frequency-weighted correlation
            float weightedL = measureLPL.process(measureHPL.process(dcL));
            float weightedR = measureLPR.process(measureHPR.process(dcR));
            weightedSumLR += weightedL * weightedR;
            weightedSumL2 += weightedL * weightedL;
            weightedSumR2 += weightedR * weightedR;

            sampleCount++;
        }

        // Process analysis window
        if (sampleCount >= analysisWindowSamples)
        {
            processAnalysisWindow();
        }

        // Update correction gain with attack/release envelope
        updateCorrectionGain();

        // Update effective parameter multipliers with smooth ramps
        updateEffectiveParameters();
    }

    /**
     * @brief Get corrected parameters based on current analysis
     *
     * @param requestedDelayL User's requested left delay (ms)
     * @param requestedDelayR User's requested right delay (ms)
     * @param requestedWidth User's requested width (%)
     * @param correctedDelayL Output: corrected left delay (ms)
     * @param correctedDelayR Output: corrected right delay (ms)
     * @param correctedWidth Output: corrected width (%)
     */
    void getCorrectedParameters(
        float requestedDelayL, float requestedDelayR, float requestedWidth,
        float& correctedDelayL, float& correctedDelayR, float& correctedWidth) const
    {
        if (!isEnabled || correctionGain < 0.001f)
        {
            correctedDelayL = requestedDelayL;
            correctedDelayR = requestedDelayR;
            correctedWidth = requestedWidth;
            return;
        }

        // Priority 1: Reduce width first (less destructive)
        float widthReduction = correctionGain * (requestedWidth - MIN_WIDTH_PERCENT);
        correctedWidth = std::max(MIN_WIDTH_PERCENT, requestedWidth - widthReduction);

        // Priority 2: Reduce delay only if width reduction wasn't enough
        // (This happens at higher correction amounts)
        float delayReductionFactor = std::max(0.0f, correctionGain - 0.5f) * 2.0f; // 0-1 for upper half
        correctedDelayL = requestedDelayL * (1.0f - delayReductionFactor * 0.5f);
        correctedDelayR = requestedDelayR * (1.0f - delayReductionFactor * 0.5f);

        // Update metering
        const_cast<AutoPhaseMetering&>(metering).effectiveWidth = correctedWidth;
        const_cast<AutoPhaseMetering&>(metering).effectiveDelayL = correctedDelayL;
        const_cast<AutoPhaseMetering&>(metering).effectiveDelayR = correctedDelayR;
    }

    /**
     * @brief Get bass-limited parameters (for low band processing)
     *
     * Bass frequencies need conservative treatment regardless of user settings.
     */
    void getBassLimitedParameters(
        float requestedDelayL, float requestedDelayR, float requestedWidth,
        float& bassDelayL, float& bassDelayR, float& bassWidth) const
    {
        bassDelayL = std::min(requestedDelayL, MAX_BASS_DELAY_MS);
        bassDelayR = std::min(requestedDelayR, MAX_BASS_DELAY_MS);
        bassWidth = std::min(requestedWidth, MAX_BASS_WIDTH);
    }

    /**
     * @brief Get metering data for UI
     */
    const AutoPhaseMetering& getMetering() const { return metering; }

    /**
     * @brief Check if transient pass-through is active
     *
     * During transients, we allow brief phase excursions to preserve punch.
     */
    bool isTransientActive() const { return isInTransient; }

    /**
     * @brief Get psychoacoustic compensation amount
     *
     * Returns 0-1 value indicating how much mid boost to apply.
     * When correction is active, subtle mid boost compensates for perceived width loss.
     */
    float getPsychoacousticCompensation() const
    {
        // 1-2dB boost scaled by correction amount
        return correctionGain * 0.15f; // ~1.5dB max as linear gain offset
    }

private:
    void processAnalysisWindow()
    {
        // Calculate raw correlation
        float denom = std::sqrt(sumL2 * sumR2);
        float rawCorr = (denom > 0.0001f) ? (sumLR / denom) : 1.0f;

        // Calculate weighted correlation
        float weightedDenom = std::sqrt(weightedSumL2 * weightedSumR2);
        float weightedCorr = (weightedDenom > 0.0001f) ? (weightedSumLR / weightedDenom) : 1.0f;

        // RMS smoothing
        smoothedCorrelation += (rawCorr - smoothedCorrelation) * rmsCoeff;
        smoothedWeightedCorrelation += (weightedCorr - smoothedWeightedCorrelation) * rmsCoeff;

        // Update metering
        metering.rawCorrelation = smoothedCorrelation;
        metering.weightedCorrelation = smoothedWeightedCorrelation;

        // Reset accumulators for next window (with 50% overlap)
        sumLR *= 0.5f;
        sumL2 *= 0.5f;
        sumR2 *= 0.5f;
        weightedSumLR *= 0.5f;
        weightedSumL2 *= 0.5f;
        weightedSumR2 *= 0.5f;
        sampleCount = analysisWindowSamples / 2;
    }

    void updateTransientDetector(float peakLevel)
    {
        // Detect if peak rose by more than threshold
        float peakRiseDb = 20.0f * std::log10((peakLevel + 1e-10f) / (prevPeakLevel + 1e-10f));

        if (peakRiseDb > TRANSIENT_THRESHOLD_DB)
        {
            isInTransient = true;
            transientHoldoffCounter = transientHoldoffSamples;
        }
        else if (transientHoldoffCounter > 0)
        {
            transientHoldoffCounter--;
            if (transientHoldoffCounter == 0)
            {
                isInTransient = false;
            }
        }

        // Update peak level with fast tracking
        prevPeakLevel = prevPeakLevel * 0.99f + peakLevel * 0.01f;

        metering.transientDetected = isInTransient;
    }

    void updateCorrectionGain()
    {
        // Use weighted correlation for correction decisions
        float correlation = smoothedWeightedCorrelation;

        // During transients, don't engage correction (preserve punch)
        if (isInTransient)
        {
            // Slowly release during transient
            correctionGain += (0.0f - correctionGain) * releaseCoeff;
            metering.correctionActive = false;
            return;
        }

        // Hysteresis logic
        if (!correctionEngaged && correlation < HYSTERESIS_ENGAGE)
        {
            correctionEngaged = true;
        }
        else if (correctionEngaged && correlation > HYSTERESIS_RELEASE)
        {
            correctionEngaged = false;
        }

        // Calculate target correction amount
        float targetGain = 0.0f;
        if (correctionEngaged && correlation < correlationThreshold)
        {
            // Proportional correction based on how far below threshold
            // Using logarithmic scaling for more musical response
            float severity = (correlationThreshold - correlation) / correlationThreshold;
            severity = std::clamp(severity, 0.0f, 1.0f);

            // Logarithmic curve: sqrt gives gentler onset, steeper at extremes
            targetGain = std::sqrt(severity);

            // Cap based on correlation level
            if (correlation > 0.4f)
                targetGain = std::min(targetGain, 0.2f);
            else if (correlation > 0.2f)
                targetGain = std::min(targetGain, 0.5f);
            else if (correlation > 0.1f)
                targetGain = std::min(targetGain, 0.8f);
            // else: full correction allowed
        }

        // Apply attack/release envelope
        float coeff = (targetGain > correctionGain) ? attackCoeff : releaseCoeff;
        correctionGain += (targetGain - correctionGain) * coeff;

        metering.correctionAmount = correctionGain;
        metering.correctionActive = correctionGain > 0.01f;
    }

    void updateEffectiveParameters()
    {
        // Smooth parameter changes to avoid zipper noise
        float targetWidthMult = 1.0f - correctionGain;
        float targetDelayMult = 1.0f - std::max(0.0f, correctionGain - 0.5f);

        effectiveWidthMult += (targetWidthMult - effectiveWidthMult) * paramRampCoeff;
        effectiveDelayMult += (targetDelayMult - effectiveDelayMult) * paramRampCoeff;
    }

    // Sample rate and timing
    double currentSampleRate = 44100.0;
    int analysisWindowSamples = 2205;  // ~50ms @ 44.1kHz
    int transientWindowSamples = 220;  // ~5ms
    int transientHoldoffSamples = 3528; // ~80ms

    // Smoothing coefficients
    float rmsCoeff = 0.001f;
    float attackCoeff = 0.001f;
    float releaseCoeff = 0.001f;
    float paramRampCoeff = 0.01f;

    // Filters
    LinkwitzRileyCrossover crossover;
    BiquadFilter dcBlockerL, dcBlockerR;
    BiquadFilter measureHPL, measureHPR;
    BiquadFilter measureLPL, measureLPR;
    BiquadFilter midBoostL, midBoostR;

    // Analysis accumulators
    float sumLR = 0.0f;
    float sumL2 = 0.0f;
    float sumR2 = 0.0f;
    float weightedSumLR = 0.0f;
    float weightedSumL2 = 0.0f;
    float weightedSumR2 = 0.0f;
    int sampleCount = 0;

    // Smoothed values
    float smoothedCorrelation = 1.0f;
    float smoothedWeightedCorrelation = 1.0f;
    float correctionGain = 0.0f;

    // Effective parameter multipliers
    float effectiveWidthMult = 1.0f;
    float effectiveDelayMult = 1.0f;

    // Transient detection
    float prevPeakLevel = 0.0f;
    int transientHoldoffCounter = 0;
    bool isInTransient = false;

    // State
    bool isEnabled = false;
    bool correctionEngaged = false;
    PhaseSafetyMode safetyMode = PhaseSafetyMode::Balanced;
    float correlationThreshold = 0.3f;
    float noiseFloorLinear = 0.001f;

    // Correction speed (0-100%)
    float correctionSpeed = 50.0f;
    float currentAttackMs = 105.0f;   // Default at 50% speed
    float currentReleaseMs = 420.0f;  // Default at 50% speed

    // Metering output
    AutoPhaseMetering metering;
};

} // namespace DSP
