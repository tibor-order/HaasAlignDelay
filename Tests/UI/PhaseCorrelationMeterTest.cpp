/*
  ==============================================================================

    PhaseCorrelationMeterTest.cpp
    Tests for Enhanced Phase Correlation Meter (Iteration 9)

    Tests verify:
    - Dimensions and scaling
    - Threshold marker behavior
    - Position indicator coloring
    - Correcting badge visibility and animation
    - Value readout formatting
    - Smoothing behavior

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/PhaseCorrelationMeter.h"

using Catch::Approx;

//==============================================================================
// T9.1 - Dimensions
//==============================================================================
TEST_CASE("T9.1 CorrelationMeter_Dimensions", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    REQUIRE(meter.getDefaultWidth() == 200);
    REQUIRE(meter.getDefaultHeight() == 24);
}

//==============================================================================
// T9.2 - Scale Range
//==============================================================================
TEST_CASE("T9.2 CorrelationMeter_ScaleRange", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    REQUIRE(meter.getScaleMin() == Approx(-1.0f));
    REQUIRE(meter.getScaleMax() == Approx(1.0f));
}

//==============================================================================
// T9.3 - Zero Position (center)
//==============================================================================
TEST_CASE("T9.3 CorrelationMeter_ZeroPosition", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setSize(200, 24);

    // 0.0 correlation should be at center (100px on 200px meter)
    float xPosition = meter.correlationToX(0.0f);
    REQUIRE(xPosition == Approx(100.0f).margin(1.0f));
}

//==============================================================================
// T9.4 - Negative One Position (left edge)
//==============================================================================
TEST_CASE("T9.4 CorrelationMeter_NegativeOnePosition", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setSize(200, 24);

    // -1.0 correlation should be at left edge (0px)
    float xPosition = meter.correlationToX(-1.0f);
    REQUIRE(xPosition == Approx(0.0f).margin(1.0f));
}

//==============================================================================
// T9.5 - Positive One Position (right edge)
//==============================================================================
TEST_CASE("T9.5 CorrelationMeter_PositiveOnePosition", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setSize(200, 24);

    // +1.0 correlation should be at right edge (200px)
    float xPosition = meter.correlationToX(1.0f);
    REQUIRE(xPosition == Approx(200.0f).margin(1.0f));
}

//==============================================================================
// T9.6 - Default Threshold
//==============================================================================
TEST_CASE("T9.6 CorrelationMeter_ThresholdDefault", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    REQUIRE(meter.getThreshold() == Approx(0.3f));
}

//==============================================================================
// T9.7 - Threshold Draggable
//==============================================================================
TEST_CASE("T9.7 CorrelationMeter_ThresholdDraggable", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setSize(200, 24);

    // Set threshold to a new value
    meter.setThreshold(0.5f);
    REQUIRE(meter.getThreshold() == Approx(0.5f));

    // Set threshold to another value
    meter.setThreshold(0.1f);
    REQUIRE(meter.getThreshold() == Approx(0.1f));
}

//==============================================================================
// T9.8 - Threshold Range (clamped 0.0-1.0)
//==============================================================================
TEST_CASE("T9.8 CorrelationMeter_ThresholdRange", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    // Test clamping at max
    meter.setThreshold(1.5f);
    REQUIRE(meter.getThreshold() == Approx(1.0f));

    // Test clamping at min
    meter.setThreshold(-0.5f);
    REQUIRE(meter.getThreshold() == Approx(0.0f));

    // Test valid range
    meter.setThreshold(0.7f);
    REQUIRE(meter.getThreshold() == Approx(0.7f));
}

//==============================================================================
// T9.9 - Indicator Color Below Threshold (red)
//==============================================================================
TEST_CASE("T9.9 CorrelationMeter_IndicatorColor_BelowThreshold", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setThreshold(0.5f);

    // Correlation below threshold
    meter.setCorrelation(0.2f);

    juce::Colour indicatorColor = meter.getIndicatorColor();
    REQUIRE(indicatorColor == ReOrderColors::statusError);  // Red: #ff3366
}

//==============================================================================
// T9.10 - Indicator Color Above Threshold (green)
//==============================================================================
TEST_CASE("T9.10 CorrelationMeter_IndicatorColor_AboveThreshold", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setThreshold(0.3f);

    // Correlation above threshold
    meter.setCorrelation(0.6f);

    // Allow smoothing to catch up
    for (int i = 0; i < 30; ++i)
        meter.updateSmoothing(0.01f);

    juce::Colour indicatorColor = meter.getIndicatorColor();
    REQUIRE(indicatorColor == ReOrderColors::statusGood);  // Green: #00ff88
}

//==============================================================================
// T9.11 - Correcting Badge Hidden
//==============================================================================
TEST_CASE("T9.11 CorrelationMeter_CorrectingBadge_Hidden", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    // Not correcting
    meter.setCorrectionActive(false);

    REQUIRE(meter.isCorrectingBadgeVisible() == false);
}

//==============================================================================
// T9.12 - Correcting Badge Visible
//==============================================================================
TEST_CASE("T9.12 CorrelationMeter_CorrectingBadge_Visible", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    // Correcting active
    meter.setCorrectionActive(true);

    REQUIRE(meter.isCorrectingBadgeVisible() == true);
}

//==============================================================================
// T9.13 - Correcting Badge Pulse
//==============================================================================
TEST_CASE("T9.13 CorrelationMeter_CorrectingBadge_Pulse", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;
    meter.setCorrectionActive(true);

    // Get initial opacity
    float opacity1 = meter.getCorrectingBadgeOpacity();

    // Simulate time passing (500ms = half cycle)
    meter.updateAnimation(0.5f);
    float opacity2 = meter.getCorrectingBadgeOpacity();

    // Simulate another 500ms (full cycle)
    meter.updateAnimation(0.5f);
    float opacity3 = meter.getCorrectingBadgeOpacity();

    // Opacity should oscillate between 1.0 and 0.5
    REQUIRE(opacity1 >= 0.5f);
    REQUIRE(opacity1 <= 1.0f);
    REQUIRE(opacity2 >= 0.5f);
    REQUIRE(opacity2 <= 1.0f);

    // After full cycle, should be back near starting point
    REQUIRE(opacity3 == Approx(opacity1).margin(0.1f));
}

//==============================================================================
// T9.14 - Value Readout Format
//==============================================================================
TEST_CASE("T9.14 CorrelationMeter_ValueReadout_Format", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    // Use immediate values for testing exact format (bypasses smoothing)

    // Positive correlation
    meter.setCorrelationImmediate(0.65f);
    REQUIRE(meter.getValueReadout() == "+0.65");

    // Negative correlation
    meter.setCorrelationImmediate(-0.32f);
    REQUIRE(meter.getValueReadout() == "-0.32");

    // Zero correlation
    meter.setCorrelationImmediate(0.0f);
    REQUIRE(meter.getValueReadout() == "+0.00");

    // Full positive
    meter.setCorrelationImmediate(1.0f);
    REQUIRE(meter.getValueReadout() == "+1.00");

    // Full negative
    meter.setCorrelationImmediate(-1.0f);
    REQUIRE(meter.getValueReadout() == "-1.00");
}

//==============================================================================
// T9.15 - Smoothing (~100ms)
//==============================================================================
TEST_CASE("T9.15 CorrelationMeter_Smoothing", "[ui][correlationmeter]")
{
    PhaseCorrelationMeter meter;

    // Set initial correlation
    meter.setCorrelation(0.0f);

    // Force display to match (for test setup)
    for (int i = 0; i < 10; ++i)
        meter.updateSmoothing(0.1f);

    // Now set a new target correlation
    meter.setCorrelation(1.0f);

    // After just 10ms, display shouldn't have reached target yet
    meter.updateSmoothing(0.01f);
    float afterShortTime = meter.getDisplayCorrelation();
    REQUIRE(afterShortTime < 0.9f);  // Should still be transitioning

    // After 200ms total, should be close to target
    for (int i = 0; i < 20; ++i)
        meter.updateSmoothing(0.01f);

    float afterLongTime = meter.getDisplayCorrelation();
    REQUIRE(afterLongTime > 0.8f);  // Should be very close to 1.0
}
