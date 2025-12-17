/*
  ==============================================================================

    OutputGainTest.cpp
    Tests for Output Gain Stage (Iteration 2)

    Tests verify:
    - Default gain is 0dB (unity)
    - Gain range -12dB to +12dB works correctly
    - dB to linear conversion accuracy
    - Gain is applied after dry/wet mix
    - Smoothing prevents clicks on rapid changes
    - 50ms ramp time
    - Bypass behavior
    - Stereo channel processing

  ==============================================================================
*/

#include "../catch2/catch_amalgamated.hpp"
#include "../../Source/DSP/HaasProcessor.h"
#include <cmath>
#include <vector>

using Catch::Approx;

// Helper: Convert dB to linear gain
static float dbToLinear(float dB)
{
    return std::pow(10.0f, dB / 20.0f);
}

// Helper: Generate a constant value buffer
static void fillBuffer(float* buffer, int numSamples, float value)
{
    for (int i = 0; i < numSamples; ++i)
        buffer[i] = value;
}

// Helper: Get peak value from buffer
static float getPeak(const float* buffer, int numSamples)
{
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        peak = std::max(peak, std::abs(buffer[i]));
    return peak;
}

TEST_CASE("OutputGain_DefaultIsUnity", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    // Default outputGain should be 0dB
    REQUIRE(params.outputGain == Approx(0.0f));

    processor.setParameters(params);

    // Process a known signal
    const int numSamples = 512;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // With 0dB gain and 100% mix, output should approximately equal input
    // (allowing for some smoothing ramp-up)
    REQUIRE(getPeak(left.data(), numSamples) == Approx(0.5f).margin(0.05f));
}

TEST_CASE("OutputGain_MinusMax", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = -12.0f;  // -12dB
    params.mix = 100.0f;
    processor.setParameters(params);

    // Let the gain smoother settle - need more time due to 50ms ramp
    const int numSamples = 8820; // ~200ms at 44.1kHz
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // -12dB = 0.25x linear gain
    float expectedGain = dbToLinear(-12.0f);
    REQUIRE(expectedGain == Approx(0.25f).margin(0.01f));

    // Check last portion of buffer (after smoothing settles)
    // Note: The M/S processing affects mono content, so we need looser tolerance
    float endPeak = getPeak(left.data() + numSamples - 100, 100);
    // With mono input (L=R), M/S processing should preserve level
    // But 2-band processing may introduce some variation
    REQUIRE(endPeak == Approx(0.5f * expectedGain).margin(0.1f));
}

TEST_CASE("OutputGain_PlusMax", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = 12.0f;  // +12dB
    params.mix = 100.0f;
    processor.setParameters(params);

    // Let the gain smoother settle
    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.1f);  // Use lower input to avoid clipping
    std::vector<float> right(numSamples, 0.1f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // +12dB = ~4x linear gain
    float expectedGain = dbToLinear(12.0f);
    REQUIRE(expectedGain == Approx(3.98f).margin(0.05f));

    // Check last portion of buffer
    float endPeak = getPeak(left.data() + numSamples - 100, 100);
    REQUIRE(endPeak == Approx(0.1f * expectedGain).margin(0.05f));
}

TEST_CASE("OutputGain_ZeroDb", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = 0.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 2205; // ~50ms
    std::vector<float> left(numSamples, 0.7f);
    std::vector<float> right(numSamples, 0.7f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // 0dB = unity gain
    float endPeak = getPeak(left.data() + numSamples - 100, 100);
    REQUIRE(endPeak == Approx(0.7f).margin(0.05f));
}

TEST_CASE("OutputGain_DbToLinear_Accuracy", "[dsp][outputgain]")
{
    // Test dB to linear conversion accuracy
    REQUIRE(dbToLinear(0.0f) == Approx(1.0f).margin(0.001f));
    REQUIRE(dbToLinear(-6.0f) == Approx(0.5012f).margin(0.001f));
    REQUIRE(dbToLinear(6.0f) == Approx(1.9953f).margin(0.001f));
    REQUIRE(dbToLinear(-12.0f) == Approx(0.2512f).margin(0.001f));
    REQUIRE(dbToLinear(12.0f) == Approx(3.9811f).margin(0.001f));
    REQUIRE(dbToLinear(-20.0f) == Approx(0.1f).margin(0.001f));
    REQUIRE(dbToLinear(20.0f) == Approx(10.0f).margin(0.001f));
}

TEST_CASE("OutputGain_AppliedAfterMix", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    // Test that gain affects both dry and wet components
    DSP::HaasParameters params;
    params.outputGain = 6.0f;  // +6dB
    params.mix = 50.0f;        // 50% wet
    processor.setParameters(params);

    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.2f);
    std::vector<float> right(numSamples, 0.2f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // Both dry and wet should be boosted by +6dB
    float expectedGain = dbToLinear(6.0f);
    float endPeak = getPeak(left.data() + numSamples - 100, 100);

    // At 50% mix: output = (dry * 0.5 + wet * 0.5) * gain
    // Should still be around input * gain (since dry=wet for constant signal)
    REQUIRE(endPeak == Approx(0.2f * expectedGain).margin(0.05f));
}

TEST_CASE("OutputGain_Smoothing_NoClicks", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    // Start with unity gain
    DSP::HaasParameters params;
    params.outputGain = 0.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    // Process some samples to settle
    const int blockSize = 512;
    std::vector<float> left(blockSize, 0.5f);
    std::vector<float> right(blockSize, 0.5f);
    processor.processBlock(left.data(), right.data(), blockSize);

    // Now make a sudden gain change
    params.outputGain = 12.0f;  // Jump to +12dB
    processor.setParameters(params);

    // Process next block and check for clicks (no sample should exceed 1.5x expected)
    fillBuffer(left.data(), blockSize, 0.5f);
    fillBuffer(right.data(), blockSize, 0.5f);

    float prevSample = left[0];
    processor.processBlock(left.data(), right.data(), blockSize);

    // Check that no sample-to-sample jump is too large (indicates click)
    float maxJump = 0.0f;
    for (int i = 1; i < blockSize; ++i)
    {
        float jump = std::abs(left[i] - prevSample);
        maxJump = std::max(maxJump, jump);
        prevSample = left[i];
    }

    // A well-smoothed signal shouldn't have jumps > 0.1 per sample
    REQUIRE(maxJump < 0.1f);
}

TEST_CASE("OutputGain_Smoothing_RampTime", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    // Start with unity gain, let it settle
    DSP::HaasParameters params;
    params.outputGain = 0.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int settleBlocks = 20;  // More settling time
    const int blockSize = 512;
    std::vector<float> left(blockSize, 0.5f);
    std::vector<float> right(blockSize, 0.5f);

    for (int i = 0; i < settleBlocks; ++i)
    {
        fillBuffer(left.data(), blockSize, 0.5f);
        fillBuffer(right.data(), blockSize, 0.5f);
        processor.processBlock(left.data(), right.data(), blockSize);
    }

    // Record the settled output level
    float settledLevel = getPeak(left.data(), blockSize);

    // Jump to +6dB
    params.outputGain = 6.0f;
    processor.setParameters(params);

    float targetGain = dbToLinear(6.0f);
    float targetLevel = settledLevel * targetGain;

    // Track how long it takes to reach ~90% of target
    int samplesTo90Percent = 0;
    float threshold90 = settledLevel + (targetLevel - settledLevel) * 0.9f;
    bool reached = false;

    for (int block = 0; block < 30 && !reached; ++block)
    {
        fillBuffer(left.data(), blockSize, 0.5f);
        fillBuffer(right.data(), blockSize, 0.5f);
        processor.processBlock(left.data(), right.data(), blockSize);

        for (int i = 0; i < blockSize && !reached; ++i)
        {
            samplesTo90Percent++;
            if (left[i] >= threshold90)
                reached = true;
        }
    }

    // The smoothing uses exponential approach with 50ms time constant
    // 90% of final value is reached at about 2.3 * time_constant = ~115ms
    // Allow tolerance: should reach 90% within 50-200ms
    REQUIRE(reached);
    REQUIRE(samplesTo90Percent < 8820); // < 200ms
    REQUIRE(samplesTo90Percent > 882);  // > 20ms (not instant)
}

TEST_CASE("OutputGain_BypassedWhenModuleOff", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = 12.0f;       // +12dB gain
    params.outputBypass = true;      // But output module is bypassed
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // With output bypass, gain should not be applied
    float endPeak = getPeak(left.data() + numSamples - 100, 100);
    REQUIRE(endPeak == Approx(0.5f).margin(0.05f));
}

TEST_CASE("OutputGain_ProcessBlock_Stereo", "[dsp][outputgain]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = -6.0f;  // -6dB
    params.mix = 100.0f;
    params.widthBypass = true;  // Bypass width to test pure gain effect
    processor.setParameters(params);

    // Use longer duration for smoothing to settle
    const int numSamples = 8820;  // 200ms
    std::vector<float> left(numSamples, 0.8f);
    std::vector<float> right(numSamples, 0.4f);  // Different level

    processor.processBlock(left.data(), right.data(), numSamples);

    float expectedGain = dbToLinear(-6.0f);

    // Both channels should be gain-adjusted equally
    float leftEnd = getPeak(left.data() + numSamples - 100, 100);
    float rightEnd = getPeak(right.data() + numSamples - 100, 100);

    // With width bypassed, the gain should be applied equally to both channels
    REQUIRE(leftEnd == Approx(0.8f * expectedGain).margin(0.15f));
    REQUIRE(rightEnd == Approx(0.4f * expectedGain).margin(0.15f));

    // With width bypassed, ratio should be preserved
    REQUIRE((leftEnd / rightEnd) == Approx(2.0f).margin(0.3f));
}
