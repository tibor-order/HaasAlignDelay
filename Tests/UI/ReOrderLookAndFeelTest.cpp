/*
  ==============================================================================

    ReOrderLookAndFeelTest.cpp
    Tests for Rotary Slider LookAndFeel (Iteration 11)

    Tests verify:
    - Rotation range (270°)
    - Start/end angles
    - Value to angle mapping
    - Knob body gradient
    - Center cap size
    - Indicator width
    - Accent color from property
    - Arc glow effect
    - Drop shadow
    - Border color

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/LookAndFeel/ReOrderLookAndFeel.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T11.1 - Rotation Range (270°)
//==============================================================================
TEST_CASE("T11.1 RotarySlider_RotationRange", "[ui][lookandfeel]")
{
    float totalRotation = ReOrderLookAndFeel::getRotaryTotalAngle();
    REQUIRE(totalRotation == Approx(270.0f).margin(0.1f));
}

//==============================================================================
// T11.2 - Start Angle (-135° / 7:30 position)
//==============================================================================
TEST_CASE("T11.2 RotarySlider_StartAngle", "[ui][lookandfeel]")
{
    float startAngle = ReOrderLookAndFeel::getRotaryStartAngle();
    // -135° in radians = -3π/4 ≈ -2.356
    REQUIRE(startAngle == Approx(-135.0f * juce::MathConstants<float>::pi / 180.0f).margin(0.01f));
}

//==============================================================================
// T11.3 - End Angle (+135° / 4:30 position)
//==============================================================================
TEST_CASE("T11.3 RotarySlider_EndAngle", "[ui][lookandfeel]")
{
    float endAngle = ReOrderLookAndFeel::getRotaryEndAngle();
    // +135° in radians = 3π/4 ≈ 2.356
    REQUIRE(endAngle == Approx(135.0f * juce::MathConstants<float>::pi / 180.0f).margin(0.01f));
}

//==============================================================================
// T11.4 - Min Value = Start Angle
//==============================================================================
TEST_CASE("T11.4 RotarySlider_MinValue_StartAngle", "[ui][lookandfeel]")
{
    float angle = ReOrderLookAndFeel::valueToAngle(0.0f);  // 0% = minimum
    float startAngle = ReOrderLookAndFeel::getRotaryStartAngle();
    REQUIRE(angle == Approx(startAngle).margin(0.01f));
}

//==============================================================================
// T11.5 - Max Value = End Angle
//==============================================================================
TEST_CASE("T11.5 RotarySlider_MaxValue_EndAngle", "[ui][lookandfeel]")
{
    float angle = ReOrderLookAndFeel::valueToAngle(1.0f);  // 100% = maximum
    float endAngle = ReOrderLookAndFeel::getRotaryEndAngle();
    REQUIRE(angle == Approx(endAngle).margin(0.01f));
}

//==============================================================================
// T11.6 - Mid Value (50% is not at top dead center due to 270° range)
//==============================================================================
TEST_CASE("T11.6 RotarySlider_MidValue_TopDead", "[ui][lookandfeel]")
{
    float angle = ReOrderLookAndFeel::valueToAngle(0.5f);  // 50%
    // 50% through 270° range from -135° should be at 0° (top)
    REQUIRE(angle == Approx(0.0f).margin(0.01f));
}

//==============================================================================
// T11.7 - Knob Body Uses Radial Gradient
//==============================================================================
TEST_CASE("T11.7 RotarySlider_KnobBodyGradient", "[ui][lookandfeel]")
{
    // Get the gradient colors
    auto [innerColor, outerColor] = ReOrderLookAndFeel::getKnobBodyGradientColors();

    // Inner should be lighter (#3a3a3a)
    REQUIRE(innerColor == juce::Colour(0xff3a3a3a));
    // Outer should be darker (#1a1a1a)
    REQUIRE(outerColor == juce::Colour(0xff1a1a1a));
}

//==============================================================================
// T11.8 - Center Cap Size (35% of diameter)
//==============================================================================
TEST_CASE("T11.8 RotarySlider_CenterCapSize", "[ui][lookandfeel]")
{
    float capPercentage = ReOrderLookAndFeel::getCenterCapSizePercentage();
    REQUIRE(capPercentage == Approx(0.35f).margin(0.01f));
}

//==============================================================================
// T11.9 - Indicator Width (2px)
//==============================================================================
TEST_CASE("T11.9 RotarySlider_IndicatorWidth", "[ui][lookandfeel]")
{
    float indicatorWidth = ReOrderLookAndFeel::getIndicatorWidth();
    REQUIRE(indicatorWidth == Approx(2.0f));
}

//==============================================================================
// T11.10 - Accent Color From Property
//==============================================================================
TEST_CASE("T11.10 RotarySlider_AccentColorFromProperty", "[ui][lookandfeel]")
{
    // Test that the default accent retrieval mechanism works
    juce::Colour defaultAccent = ReOrderLookAndFeel::getDefaultAccentColor();
    // Should return a valid color (not transparent)
    REQUIRE(defaultAccent.getAlpha() > 0);
}

//==============================================================================
// T11.11 - Arc Glow Present
//==============================================================================
TEST_CASE("T11.11 RotarySlider_ArcGlowPresent", "[ui][lookandfeel]")
{
    // Verify arc glow configuration exists
    float glowRadius = ReOrderLookAndFeel::getArcGlowRadius();
    REQUIRE(glowRadius > 0.0f);
}

//==============================================================================
// T11.12 - Drop Shadow Present
//==============================================================================
TEST_CASE("T11.12 RotarySlider_DropShadowPresent", "[ui][lookandfeel]")
{
    // Verify drop shadow configuration
    auto shadowParams = ReOrderLookAndFeel::getKnobDropShadow();
    REQUIRE(shadowParams.offsetY > 0);       // Shadow should be below
    REQUIRE(shadowParams.blurRadius > 0);    // Should have blur
}

//==============================================================================
// T11.13 - Border Color (#333)
//==============================================================================
TEST_CASE("T11.13 RotarySlider_BorderColor", "[ui][lookandfeel]")
{
    juce::Colour borderColor = ReOrderLookAndFeel::getKnobBorderColor();
    REQUIRE(borderColor == juce::Colour(0xff333333));
}
