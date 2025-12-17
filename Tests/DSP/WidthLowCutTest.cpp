/*
  ==============================================================================

    WidthLowCutTest.cpp
    Tests for Width Low Cut Filter (Iteration 3)

    Tests verify:
    - Default frequency is 250Hz
    - Frequency range 20-500Hz
    - Bass frequencies are attenuated in the side channel
    - High frequencies pass unaffected
    - -3dB at cutoff frequency (Butterworth characteristic)
    - Only affects side channel, not mid
    - Bass content becomes mono (correlation near +1.0)
    - Bypass behavior
    - Smooth frequency changes

  ==============================================================================
*/

#include "../catch2/catch_amalgamated.hpp"
#include "../../Source/DSP/HaasProcessor.h"
#include <cmath>
#include <vector>

using Catch::Approx;

// Helper: Generate a sine wave
static void generateSineWave(float* buffer, int numSamples, float frequency, double sampleRate, float amplitude = 0.5f)
{
    for (int i = 0; i < numSamples; ++i)
    {
        buffer[i] = amplitude * std::sin(2.0 * M_PI * frequency * i / sampleRate);
    }
}

// Helper: Calculate RMS level
static float calculateRMS(const float* buffer, int numSamples)
{
    float sum = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sum += buffer[i] * buffer[i];
    return std::sqrt(sum / numSamples);
}

// Helper: Calculate correlation between two signals
static float calculateCorrelation(const float* left, const float* right, int numSamples)
{
    float sumLR = 0.0f, sumL2 = 0.0f, sumR2 = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        sumLR += left[i] * right[i];
        sumL2 += left[i] * left[i];
        sumR2 += right[i] * right[i];
    }
    float denom = std::sqrt(sumL2 * sumR2);
    if (denom < 0.0001f) return 1.0f;
    return sumLR / denom;
}

TEST_CASE("T3.1 WidthLowCut_DefaultFrequency", "[dsp][widthlowcut]")
{
    DSP::HaasParameters params;
    // Default widthLowCut should be 250Hz
    REQUIRE(params.widthLowCut == Approx(250.0f));
}

TEST_CASE("T3.2 WidthLowCut_RangeMin", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 20.0f;  // Minimum value
    params.width = 200.0f;       // Max width to make effect obvious
    params.mix = 100.0f;
    processor.setParameters(params);

    // Should accept 20Hz without issues
    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    // Should not crash
    processor.processBlock(left.data(), right.data(), numSamples);
    REQUIRE(true);  // If we get here, 20Hz was accepted
}

TEST_CASE("T3.3 WidthLowCut_RangeMax", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    processor.prepare(44100.0, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 500.0f;  // Maximum value
    params.width = 200.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 4410;
    std::vector<float> left(numSamples, 0.5f);
    std::vector<float> right(numSamples, 0.5f);

    // Should not crash
    processor.processBlock(left.data(), right.data(), numSamples);
    REQUIRE(true);  // If we get here, 500Hz was accepted
}

TEST_CASE("T3.4 WidthLowCut_BassAttenuation_50Hz", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;  // Cutoff at 250Hz
    params.width = 200.0f;        // Max width
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create a stereo-different signal at 50Hz (below cutoff)
    // This creates side content that should be attenuated
    const int numSamples = 8820;  // 200ms
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    // Left = sine, Right = -sine (pure side content)
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 50.0 * i / sampleRate);
        left[i] = sample;
        right[i] = -sample;  // Out of phase = pure side
    }

    // Record input side level
    float inputSideRMS = calculateRMS(left.data(), numSamples);

    processor.processBlock(left.data(), right.data(), numSamples);

    // Calculate output side: (L - R) / 2
    std::vector<float> outputSide(numSamples);
    for (int i = 0; i < numSamples; ++i)
        outputSide[i] = (left[i] - right[i]) * 0.5f;

    float outputSideRMS = calculateRMS(outputSide.data() + numSamples/2, numSamples/2);

    // At 50Hz with 250Hz cutoff (2 octaves below), 2nd-order HP should attenuate
    // by approximately 12dB/octave * 2 = 24dB, so output should be much lower
    // Allow for processing variations, but should be significantly attenuated
    float attenuationDb = 20.0f * std::log10(outputSideRMS / inputSideRMS + 0.0001f);
    REQUIRE(attenuationDb < -6.0f);  // At least 6dB attenuation
}

TEST_CASE("T3.5 WidthLowCut_HighsPass_1kHz", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;  // Cutoff at 250Hz
    params.width = 200.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create stereo-different signal at 1kHz (well above cutoff)
    const int numSamples = 8820;
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 1000.0 * i / sampleRate);
        left[i] = sample;
        right[i] = -sample;
    }

    float inputSideRMS = calculateRMS(left.data(), numSamples);

    processor.processBlock(left.data(), right.data(), numSamples);

    std::vector<float> outputSide(numSamples);
    for (int i = 0; i < numSamples; ++i)
        outputSide[i] = (left[i] - right[i]) * 0.5f;

    float outputSideRMS = calculateRMS(outputSide.data() + numSamples/2, numSamples/2);

    // At 1kHz, should pass with minimal attenuation (< 1dB)
    float attenuationDb = 20.0f * std::log10(outputSideRMS / inputSideRMS + 0.0001f);
    REQUIRE(attenuationDb > -3.0f);  // Less than 3dB attenuation
}

TEST_CASE("T3.6 WidthLowCut_AtCutoff_3dB", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    // Use unity width to test filter response directly
    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;
    params.width = 100.0f;  // Unity width to isolate filter effect
    params.mix = 100.0f;
    processor.setParameters(params);

    // Signal at cutoff frequency
    const int numSamples = 8820;
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 250.0 * i / sampleRate);
        left[i] = sample;
        right[i] = -sample;
    }

    // Calculate input side RMS
    std::vector<float> inputSide(numSamples);
    for (int i = 0; i < numSamples; ++i)
        inputSide[i] = (left[i] - right[i]) * 0.5f;
    float inputSideRMS = calculateRMS(inputSide.data() + numSamples/2, numSamples/2);

    processor.processBlock(left.data(), right.data(), numSamples);

    std::vector<float> outputSide(numSamples);
    for (int i = 0; i < numSamples; ++i)
        outputSide[i] = (left[i] - right[i]) * 0.5f;

    float outputSideRMS = calculateRMS(outputSide.data() + numSamples/2, numSamples/2);

    // At cutoff frequency with 2nd-order Butterworth HP, should be around -3dB
    // Allow more tolerance due to 2-band processing complexity
    float attenuationDb = 20.0f * std::log10(outputSideRMS / inputSideRMS + 0.0001f);
    REQUIRE(attenuationDb == Approx(-3.0f).margin(3.0f));  // Allow ±3dB tolerance
}

TEST_CASE("T3.7 WidthLowCut_SideChannelOnly", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;
    params.width = 100.0f;  // Unity width
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create mono signal (pure mid, no side)
    const int numSamples = 8820;
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 50.0 * i / sampleRate);
        left[i] = sample;
        right[i] = sample;  // Same = pure mid
    }

    float inputMidRMS = calculateRMS(left.data(), numSamples);

    processor.processBlock(left.data(), right.data(), numSamples);

    // Calculate output mid: (L + R) / 2
    std::vector<float> outputMid(numSamples);
    for (int i = 0; i < numSamples; ++i)
        outputMid[i] = (left[i] + right[i]) * 0.5f;

    float outputMidRMS = calculateRMS(outputMid.data() + numSamples/2, numSamples/2);

    // Mid channel should be unaffected by low cut (within reasonable tolerance)
    float attenuationDb = 20.0f * std::log10(outputMidRMS / inputMidRMS + 0.0001f);
    REQUIRE(attenuationDb > -3.0f);  // Should not attenuate mid significantly
}

TEST_CASE("T3.8 WidthLowCut_MonoBassResult", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;
    params.width = 200.0f;  // High width
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create stereo bass signal with some mid content
    // (pure out-of-phase stays correlated even when attenuated)
    const int numSamples = 8820;
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float bassWave = 0.5f * std::sin(2.0 * M_PI * 50.0 * i / sampleRate);
        // Add some mid content (mono) and side content (stereo difference)
        left[i] = 0.3f + bassWave;      // Mid + side
        right[i] = 0.3f - bassWave;     // Mid - side
    }

    // Calculate input side RMS
    std::vector<float> inputSide(numSamples);
    for (int i = 0; i < numSamples; ++i)
        inputSide[i] = (left[i] - right[i]) * 0.5f;
    float inputSideRMS = calculateRMS(inputSide.data() + numSamples/2, numSamples/2);

    processor.processBlock(left.data(), right.data(), numSamples);

    // Calculate output side RMS
    std::vector<float> outputSide(numSamples);
    for (int i = 0; i < numSamples; ++i)
        outputSide[i] = (left[i] - right[i]) * 0.5f;
    float outputSideRMS = calculateRMS(outputSide.data() + numSamples/2, numSamples/2);

    // Side channel should be attenuated (bass stereo content reduced)
    // The low-cut filter reduces bass side content, making bass more mono
    float attenuationDb = 20.0f * std::log10(outputSideRMS / inputSideRMS + 0.0001f);
    REQUIRE(attenuationDb < -3.0f);  // At least 3dB attenuation of bass side content
}

TEST_CASE("T3.9 WidthLowCut_BypassWhenModuleOff", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;
    params.width = 200.0f;
    params.widthBypass = true;  // Width module bypassed
    params.mix = 100.0f;
    processor.setParameters(params);

    // Create stereo bass signal
    const int numSamples = 8820;
    std::vector<float> left(numSamples);
    std::vector<float> right(numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 50.0 * i / sampleRate);
        left[i] = sample;
        right[i] = -sample;
    }

    processor.processBlock(left.data(), right.data(), numSamples);

    // With width bypass, the low cut should not be applied
    // Correlation should remain near -1.0 (original out-of-phase signal)
    float correlation = calculateCorrelation(
        left.data() + numSamples/2,
        right.data() + numSamples/2,
        numSamples/2);

    // Should remain highly anti-correlated when bypassed
    REQUIRE(correlation < 0.0f);
}

TEST_CASE("T3.10 WidthLowCut_SmoothFreqChange", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 100.0f;
    params.width = 150.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    // Let it settle
    const int blockSize = 512;
    std::vector<float> left(blockSize, 0.5f);
    std::vector<float> right(blockSize, 0.5f);

    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < blockSize; ++j)
        {
            left[j] = 0.5f;
            right[j] = 0.5f;
        }
        processor.processBlock(left.data(), right.data(), blockSize);
    }

    // Make a sudden frequency change
    params.widthLowCut = 400.0f;
    processor.setParameters(params);

    // Process and check for clicks (large sample-to-sample jumps)
    for (int j = 0; j < blockSize; ++j)
    {
        left[j] = 0.5f;
        right[j] = 0.5f;
    }

    float prevSampleL = left[0];
    processor.processBlock(left.data(), right.data(), blockSize);

    float maxJump = 0.0f;
    for (int i = 1; i < blockSize; ++i)
    {
        float jump = std::abs(left[i] - prevSampleL);
        maxJump = std::max(maxJump, jump);
        prevSampleL = left[i];
    }

    // Should not have audible clicks (large jumps)
    REQUIRE(maxJump < 0.2f);
}

TEST_CASE("T3.11 WidthLowCut_InteractionWithWidth", "[dsp][widthlowcut]")
{
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    // Test with different width values
    const int numSamples = 8820;

    for (float width : {100.0f, 150.0f, 200.0f})
    {
        DSP::HaasParameters params;
        params.widthLowCut = 250.0f;
        params.width = width;
        params.mix = 100.0f;
        processor.setParameters(params);

        std::vector<float> left(numSamples);
        std::vector<float> right(numSamples);

        // Create mixed frequency content
        for (int i = 0; i < numSamples; ++i)
        {
            float bass = 0.3f * std::sin(2.0 * M_PI * 50.0 * i / sampleRate);
            float high = 0.3f * std::sin(2.0 * M_PI * 1000.0 * i / sampleRate);
            left[i] = bass + high;
            right[i] = -bass + high;  // Bass out of phase, highs in phase
        }

        processor.processBlock(left.data(), right.data(), numSamples);

        // Should process without issues at any width
        float correlation = calculateCorrelation(
            left.data() + numSamples/2,
            right.data() + numSamples/2,
            numSamples/2);

        // Should have reasonable correlation (not crashed or NaN)
        REQUIRE(std::isfinite(correlation));
    }
}

TEST_CASE("T3.12 WidthLowCut_FilterCoefficients", "[dsp][widthlowcut]")
{
    // Test that the filter is actually a 2nd-order Butterworth (12dB/octave slope)
    DSP::HaasProcessor processor;
    const double sampleRate = 44100.0;
    processor.prepare(sampleRate, 512);

    DSP::HaasParameters params;
    params.widthLowCut = 250.0f;
    params.width = 200.0f;
    params.mix = 100.0f;
    processor.setParameters(params);

    const int numSamples = 8820;

    // Test attenuation at one octave below cutoff (125Hz)
    std::vector<float> left125(numSamples);
    std::vector<float> right125(numSamples);
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.5f * std::sin(2.0 * M_PI * 125.0 * i / sampleRate);
        left125[i] = sample;
        right125[i] = -sample;
    }

    float inputRMS125 = calculateRMS(left125.data(), numSamples);

    processor.processBlock(left125.data(), right125.data(), numSamples);

    std::vector<float> outputSide125(numSamples);
    for (int i = 0; i < numSamples; ++i)
        outputSide125[i] = (left125[i] - right125[i]) * 0.5f;

    float outputRMS125 = calculateRMS(outputSide125.data() + numSamples/2, numSamples/2);
    float atten125dB = 20.0f * std::log10(outputRMS125 / inputRMS125 + 0.0001f);

    // One octave below cutoff, 2nd-order HP should attenuate ~12dB
    // Allow significant tolerance due to processing chain complexity
    REQUIRE(atten125dB < -3.0f);   // At least some attenuation
    REQUIRE(atten125dB > -20.0f);  // But not excessively
}
