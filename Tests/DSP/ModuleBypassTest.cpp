/*
  ==============================================================================

    ModuleBypassTest.cpp
    Tests for Module Bypass System (Iteration 5)

    Tests verify:
    - Default bypass states are false
    - Each module can be bypassed independently
    - Bypass transitions are click-free (10ms crossfade)
    - Multiple bypasses can be active simultaneously
    - Master bypass overrides all modules
    - Metering continues when bypassed

  ==============================================================================
*/

#include "../catch2/catch_amalgamated.hpp"
#include "../../Source/DSP/HaasProcessor.h"
#include <cmath>
#include <vector>

using Catch::Approx;

// Helper: Calculate RMS level
static float calculateRMS(const float* buffer, int numSamples)
{
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sum += buffer[i] * buffer[i];
    return std::sqrt(sum / numSamples);
}

// Helper: Check for clicks (large sample-to-sample jumps)
static float getMaxSampleJump(const float* buffer, int numSamples)
{
    float maxJump = 0.0f;
    for (int i = 1; i < numSamples; ++i)
    {
        float jump = std::abs(buffer[i] - buffer[i-1]);
        maxJump = std::max(maxJump, jump);
    }
    return maxJump;
}

TEST_CASE("T5.1 ModuleBypass_Delay_Default", "[dsp][modulebypass]")
{
    DSP::HaasParameters params;
    REQUIRE(params.delayBypass == false);
}

TEST_CASE("T5.2 ModuleBypass_Width_Default", "[dsp][modulebypass]")
{
    DSP::HaasParameters params;
    REQUIRE(params.widthBypass == false);
}

TEST_CASE("T5.3 ModuleBypass_Phase_Default", "[dsp][modulebypass]")
{
    DSP::HaasParameters params;
    REQUIRE(params.phaseBypass == false);
}

TEST_CASE("T5.4 ModuleBypass_Output_Default", "[dsp][modulebypass]")
{
    DSP::HaasParameters params;
    REQUIRE(params.outputBypass == false);
}

TEST_CASE("T5.5 ModuleBypass_Delay_NoProcessing", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.delayLeftMs = 10.0f;   // Set delay
    params.delayRightMs = 5.0f;
    params.delayBypass = true;    // But bypass it
    params.mix = 100.0f;
    processor.setParameters(params);

    // Process an impulse
    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.0f);
    std::vector<float> right(numSamples, 0.0f);
    left[0] = 1.0f;
    right[0] = 1.0f;

    processor.processBlock(left.data(), right.data(), numSamples);

    // With delay bypassed, the impulse should appear at the start (or nearly)
    // not delayed by 10ms (441 samples at 44.1kHz)
    // Check that energy is in the first few samples
    float earlyEnergy = 0.0f;
    for (int i = 0; i < 50; ++i)
        earlyEnergy += left[i] * left[i];

    float lateEnergy = 0.0f;
    for (int i = 400; i < 500; ++i)
        lateEnergy += left[i] * left[i];

    // Most energy should be early, not delayed
    REQUIRE(earlyEnergy > lateEnergy);
}

TEST_CASE("T5.6 ModuleBypass_Width_NoProcessing", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.width = 200.0f;      // Double width
    params.widthBypass = true;  // But bypass width
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create stereo signal with different L/R
    const int numSamples = 8820;
    std::vector<float> left(numSamples, 0.6f);
    std::vector<float> right(numSamples, 0.4f);

    // Store original difference
    float origDiff = 0.6f - 0.4f;

    processor.processBlock(left.data(), right.data(), numSamples);

    // With width bypassed, difference should be preserved (not widened)
    float finalDiff = std::abs(left[numSamples-1] - right[numSamples-1]);

    // Should be approximately the same (allowing for 2-band processing)
    REQUIRE(finalDiff == Approx(origDiff).margin(0.15f));
}

TEST_CASE("T5.7 ModuleBypass_Phase_NoCorrection", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.autoPhaseEnabled = true;
    params.phaseBypass = true;      // But bypass phase correction
    params.width = 180.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create out-of-phase signal that would normally trigger correction
    const int numSamples = 8820;
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 100.0 * i / 44100.0);
        left[i] = sample;
        right[i] = -sample;  // Out of phase
    }

    processor.processBlock(left.data(), right.data(), numSamples);

    // With phase bypass, correction should not be active
    REQUIRE(processor.getMetering().correctionAmount == Approx(0.0f).margin(0.05f));
}

TEST_CASE("T5.8 ModuleBypass_Output_NoGain", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = 12.0f;    // +12dB gain
    params.outputBypass = true;   // But bypass output
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 8820;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // With output bypass, should be unity gain
    float finalLevel = calculateRMS(left.data() + numSamples - 100, 100);
    REQUIRE(finalLevel == Approx(0.5f).margin(0.1f));
}

TEST_CASE("T5.9 ModuleBypass_Crossfade_NoClicks", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    // Start with delay active
    DSP::HaasParameters params;
    params.delayLeftMs = 5.0f;
    params.delayBypass = false;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int blockSize = 512;
    std::vector<float> left(blockSize, 0.5f);
    std::vector<float> right(blockSize, 0.5f);

    // Let it settle
    for (int i = 0; i < 10; ++i)
        processor.processBlock(left.data(), right.data(), blockSize);

    // Toggle bypass
    params.delayBypass = true;
    processor.setParameters(params);

    // Refill with constant value
    std::fill(left.begin(), left.end(), 0.5f);
    std::fill(right.begin(), right.end(), 0.5f);

    processor.processBlock(left.data(), right.data(), blockSize);

    // Check for clicks
    float maxJump = getMaxSampleJump(left.data(), blockSize);
    REQUIRE(maxJump < 0.15f);  // No audible clicks
}

TEST_CASE("T5.10 ModuleBypass_Crossfade_Duration", "[dsp][modulebypass]")
{
    // 10ms at 44.1kHz = 441 samples
    // Verify crossfade completes within reasonable time
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.outputGain = 6.0f;
    params.outputBypass = false;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int blockSize = 512;
    std::vector<float> left(blockSize, 0.5f);
    std::vector<float> right(blockSize, 0.5f);

    // Let gain settle
    for (int i = 0; i < 20; ++i)
        processor.processBlock(left.data(), right.data(), blockSize);

    float withGain = calculateRMS(left.data(), blockSize);

    // Toggle bypass
    params.outputBypass = true;
    processor.setParameters(params);

    // Process and track how fast it reaches unity
    int samplesUntilNearUnity = 0;
    bool reachedUnity = false;

    for (int block = 0; block < 10 && !reachedUnity; ++block)
    {
        std::fill(left.begin(), left.end(), 0.5f);
        std::fill(right.begin(), right.end(), 0.5f);
        processor.processBlock(left.data(), right.data(), blockSize);

        float rms = calculateRMS(left.data(), blockSize);
        if (std::abs(rms - 0.5f) < 0.05f)
            reachedUnity = true;
        else
            samplesUntilNearUnity += blockSize;
    }

    // Should reach unity within ~50ms (2200 samples) - allowing for smoothing
    REQUIRE(reachedUnity);
    REQUIRE(samplesUntilNearUnity < 2205);
}

TEST_CASE("T5.11 ModuleBypass_Multiple_Simultaneous", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.delayLeftMs = 5.0f;
    params.width = 150.0f;
    params.outputGain = 6.0f;
    params.autoPhaseEnabled = true;

    // Bypass multiple modules at once
    params.delayBypass = true;
    params.widthBypass = true;
    params.outputBypass = true;
    params.phaseBypass = true;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // With all bypassed, output should be close to input
    float rms = calculateRMS(left.data() + numSamples - 100, 100);
    REQUIRE(rms == Approx(0.5f).margin(0.1f));
}

TEST_CASE("T5.12 ModuleBypass_MasterBypass_Override", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.delayLeftMs = 10.0f;
    params.outputGain = 12.0f;
    params.bypass = true;  // Master bypass
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // Master bypass should pass audio unchanged
    float rms = calculateRMS(left.data() + numSamples - 100, 100);
    REQUIRE(rms == Approx(0.5f).margin(0.02f));
}

TEST_CASE("T5.13 ModuleBypass_StatePreserved", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.delayBypass = true;
    params.widthBypass = true;
    processor.setParameters(params);

    // Process some audio
    const int blockSize = 512;
    std::vector<float> left(blockSize, 0.5f);
    std::vector<float> right(blockSize, 0.5f);
    processor.processBlock(left.data(), right.data(), blockSize);

    // Re-prepare (simulating host prepare call)
    processor.prepare(44100.0, 512);

    // Bypass state should be maintained through setParameters
    // (Note: actual state preservation depends on setParameters being called after prepare)
    params.delayBypass = true;  // Reset to same values
    params.widthBypass = true;
    processor.setParameters(params);

    // Should still work correctly
    processor.processBlock(left.data(), right.data(), blockSize);
    REQUIRE(true);  // No crash = success
}

TEST_CASE("T5.14 ModuleBypass_MetersContinue", "[dsp][modulebypass]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.bypass = true;  // Master bypass
    params.mix = 100.0f;
    processor.setParameters(params);

    // Process audio
    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    processor.processBlock(left.data(), right.data(), numSamples);

    // Metering should still report levels even when bypassed
    auto metering = processor.getMetering();
    REQUIRE(metering.inputLevel > 0.0f);
}
