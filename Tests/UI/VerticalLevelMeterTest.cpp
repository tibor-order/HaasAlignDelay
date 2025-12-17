/*
  ==============================================================================

    VerticalLevelMeterTest.cpp
    Tests for Vertical Level Meter Component (Iteration 8)

    Tests verify:
    - 24 segments with correct dimensions
    - Color zones (green, yellow, red)
    - Level decay and peak hold
    - Value clamping

  ==============================================================================
*/

#include "../catch2/catch_amalgamated.hpp"
#include "../../Source/UI/VerticalLevelMeter.h"

using Catch::Approx;

TEST_CASE("T8.1 LevelMeter_SegmentCount", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    REQUIRE(meter.getSegmentCount() == 24);
}

TEST_CASE("T8.2 LevelMeter_LevelZero_NoSegments", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.setLevel(0.0f);
    REQUIRE(meter.getLitSegmentCount() == 0);
}

TEST_CASE("T8.3 LevelMeter_LevelHalf_12Segments", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.setLevel(0.5f);
    REQUIRE(meter.getLitSegmentCount() == 12);
}

TEST_CASE("T8.4 LevelMeter_LevelFull_24Segments", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.setLevel(1.0f);
    REQUIRE(meter.getLitSegmentCount() == 24);
}

TEST_CASE("T8.5 LevelMeter_ColorZone_Green", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    // Segments 1-14 (0-13 in 0-indexed) should be green (0-60% of 24 = ~14.4)
    // Actually 60% of 24 = 14.4, so segments 0-13 are green
    for (int i = 0; i < 14; ++i)
    {
        auto color = meter.getSegmentColor(i);
        // Green zone color: #00ff88
        REQUIRE(color.getGreen() > color.getRed());
        REQUIRE(color.getGreen() > 200);
    }
}

TEST_CASE("T8.6 LevelMeter_ColorZone_Yellow", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    // Segments 15-18 (14-17 in 0-indexed) should be yellow (60-75% of 24)
    // 60% = 14.4, 75% = 18
    for (int i = 14; i < 18; ++i)
    {
        auto color = meter.getSegmentColor(i);
        // Yellow zone color: #ffcc00
        REQUIRE(color.getRed() > 200);
        REQUIRE(color.getGreen() > 150);
        REQUIRE(color.getBlue() < 100);
    }
}

TEST_CASE("T8.7 LevelMeter_ColorZone_Red", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    // Segments 19-24 (18-23 in 0-indexed) should be red (75-100%)
    for (int i = 18; i < 24; ++i)
    {
        auto color = meter.getSegmentColor(i);
        // Red zone color: #ff3366
        REQUIRE(color.getRed() > 200);
        REQUIRE(color.getGreen() < 100);
    }
}

TEST_CASE("T8.8 LevelMeter_Decay_TimeConstant", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.prepare(44100.0);  // Set sample rate for timing

    // Set level to full
    meter.setLevel(1.0f);
    REQUIRE(meter.getDisplayLevel() == Approx(1.0f).margin(0.01f));

    // Set level to zero and simulate ~50ms of decay
    meter.setLevel(0.0f);

    // Simulate 50ms worth of updates (at ~60Hz refresh = ~3 frames)
    // After 50ms (one time constant), level should decay to ~37% (1/e)
    for (int i = 0; i < 3; ++i)
        meter.updateDecay(1.0f / 60.0f);  // 60Hz updates

    float decayedLevel = meter.getDisplayLevel();
    // Should be roughly 37% of original (e^-1 ≈ 0.368)
    // Allow wide tolerance due to discrete updates
    REQUIRE(decayedLevel < 0.7f);
    REQUIRE(decayedLevel > 0.1f);
}

TEST_CASE("T8.9 LevelMeter_PeakHold_Duration", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.prepare(44100.0);

    // Set peak
    meter.setLevel(1.0f);
    float initialPeak = meter.getPeakLevel();
    REQUIRE(initialPeak == Approx(1.0f));

    // Drop level
    meter.setLevel(0.0f);

    // After 1 second, peak should still be held
    for (int i = 0; i < 60; ++i)  // 60 frames at 60Hz = 1 second
        meter.updateDecay(1.0f / 60.0f);

    REQUIRE(meter.getPeakLevel() == Approx(1.0f));

    // After 2+ seconds total, peak should start to fall
    for (int i = 0; i < 70; ++i)  // Another 1.17 seconds
        meter.updateDecay(1.0f / 60.0f);

    REQUIRE(meter.getPeakLevel() < 1.0f);
}

TEST_CASE("T8.10 LevelMeter_PeakHold_Position", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.prepare(44100.0);

    // Set varying levels
    meter.setLevel(0.5f);
    meter.setLevel(0.8f);  // New peak
    meter.setLevel(0.3f);

    // Peak should be at the highest point reached
    REQUIRE(meter.getPeakLevel() == Approx(0.8f));
}

TEST_CASE("T8.11 LevelMeter_ClampAboveOne", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.setLevel(1.5f);  // Above 1.0
    REQUIRE(meter.getDisplayLevel() == Approx(1.0f));
    REQUIRE(meter.getLitSegmentCount() == 24);
}

TEST_CASE("T8.12 LevelMeter_ClampBelowZero", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    meter.setLevel(-0.5f);  // Below 0.0
    REQUIRE(meter.getDisplayLevel() == Approx(0.0f));
    REQUIRE(meter.getLitSegmentCount() == 0);
}

TEST_CASE("T8.13 LevelMeter_SegmentDimensions", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    REQUIRE(meter.getSegmentWidth() == 8);
    REQUIRE(meter.getSegmentHeight() == 3);
}

TEST_CASE("T8.14 LevelMeter_SegmentGap", "[ui][levelmeter]")
{
    VerticalLevelMeter meter;
    REQUIRE(meter.getSegmentGap() == 2);
}
