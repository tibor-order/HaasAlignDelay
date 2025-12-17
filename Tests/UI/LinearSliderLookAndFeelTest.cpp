/*
  ==============================================================================

    LinearSliderLookAndFeelTest.cpp
    Tests for Linear Slider LookAndFeel (Iteration 12)

    Tests verify:
    - Vertical slider track width (12px)
    - Fill width (6px centered)
    - Fill direction (from bottom)
    - Handle size (20×16px)
    - Handle gradient
    - Glow on fill
    - Horizontal slider track height (8px)
    - Horizontal fill direction (from left)
    - Thumb size (16px)
    - Accent color from property
    - Inset shadow
    - Rounded corners (6px)

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/LookAndFeel/ReOrderLookAndFeel.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T12.1 - Vertical Slider Track Width (12px)
//==============================================================================
TEST_CASE("T12.1 VerticalSlider_TrackWidth", "[ui][linearslider]")
{
    float trackWidth = ReOrderLookAndFeel::getVerticalSliderTrackWidth();
    REQUIRE(trackWidth == Approx(12.0f));
}

//==============================================================================
// T12.2 - Vertical Slider Fill Width (6px centered)
//==============================================================================
TEST_CASE("T12.2 VerticalSlider_FillWidth", "[ui][linearslider]")
{
    float fillWidth = ReOrderLookAndFeel::getVerticalSliderFillWidth();
    REQUIRE(fillWidth == Approx(6.0f));
}

//==============================================================================
// T12.3 - Vertical Slider Fill From Bottom
//==============================================================================
TEST_CASE("T12.3 VerticalSlider_FillFromBottom", "[ui][linearslider]")
{
    bool fromBottom = ReOrderLookAndFeel::isVerticalSliderFillFromBottom();
    REQUIRE(fromBottom == true);
}

//==============================================================================
// T12.4 - Vertical Slider Handle Size (20×16px)
//==============================================================================
TEST_CASE("T12.4 VerticalSlider_HandleSize", "[ui][linearslider]")
{
    auto handleSize = ReOrderLookAndFeel::getVerticalSliderHandleSize();
    REQUIRE(handleSize.first == Approx(20.0f));   // Width
    REQUIRE(handleSize.second == Approx(16.0f));  // Height
}

//==============================================================================
// T12.5 - Vertical Slider Handle Gradient (metallic)
//==============================================================================
TEST_CASE("T12.5 VerticalSlider_HandleGradient", "[ui][linearslider]")
{
    auto [topColor, bottomColor] = ReOrderLookAndFeel::getHandleGradientColors();

    // Top should be lighter (#555)
    REQUIRE(topColor == juce::Colour(0xff555555));
    // Bottom should be darker (#333)
    REQUIRE(bottomColor == juce::Colour(0xff333333));
}

//==============================================================================
// T12.6 - Vertical Slider Glow On Fill
//==============================================================================
TEST_CASE("T12.6 VerticalSlider_GlowOnFill", "[ui][linearslider]")
{
    float glowRadius = ReOrderLookAndFeel::getLinearSliderFillGlowRadius();
    REQUIRE(glowRadius > 0.0f);
}

//==============================================================================
// T12.7 - Horizontal Slider Track Height (8px)
//==============================================================================
TEST_CASE("T12.7 HorizontalSlider_TrackHeight", "[ui][linearslider]")
{
    float trackHeight = ReOrderLookAndFeel::getHorizontalSliderTrackHeight();
    REQUIRE(trackHeight == Approx(8.0f));
}

//==============================================================================
// T12.8 - Horizontal Slider Fill From Left
//==============================================================================
TEST_CASE("T12.8 HorizontalSlider_FillFromLeft", "[ui][linearslider]")
{
    bool fromLeft = ReOrderLookAndFeel::isHorizontalSliderFillFromLeft();
    REQUIRE(fromLeft == true);
}

//==============================================================================
// T12.9 - Horizontal Slider Thumb Size (16px)
//==============================================================================
TEST_CASE("T12.9 HorizontalSlider_ThumbSize", "[ui][linearslider]")
{
    float thumbSize = ReOrderLookAndFeel::getHorizontalSliderThumbSize();
    REQUIRE(thumbSize == Approx(16.0f));
}

//==============================================================================
// T12.10 - Linear Slider Accent From Property
//==============================================================================
TEST_CASE("T12.10 LinearSlider_AccentFromProperty", "[ui][linearslider]")
{
    // Verify the default accent color is valid
    juce::Colour accentColor = ReOrderLookAndFeel::getDefaultAccentColor();
    REQUIRE(accentColor.getAlpha() > 0);
}

//==============================================================================
// T12.11 - Linear Slider Inset Shadow
//==============================================================================
TEST_CASE("T12.11 LinearSlider_InsetShadow", "[ui][linearslider]")
{
    bool hasInsetShadow = ReOrderLookAndFeel::hasLinearSliderInsetShadow();
    REQUIRE(hasInsetShadow == true);
}

//==============================================================================
// T12.12 - Linear Slider Rounded Corners (6px)
//==============================================================================
TEST_CASE("T12.12 LinearSlider_RoundedCorners", "[ui][linearslider]")
{
    float cornerRadius = ReOrderLookAndFeel::getLinearSliderCornerRadius();
    REQUIRE(cornerRadius == Approx(6.0f));
}
