/*
  ==============================================================================

    ButtonLookAndFeelTest.cpp
    Tests for Button LookAndFeel (Iteration 13)

    Tests verify:
    - Power button size (20×20px)
    - Power button off border color (#444)
    - Power button on border color (accent)
    - Power button off fill (transparent)
    - Power button on fill (20% accent)
    - Toggle button off background (#1a1a1a)
    - Toggle button off text color (#666)
    - Toggle button on gradient fill
    - Toggle button on glow
    - Toggle button border radius (4-6px)
    - Bypass button active (gray)
    - Bypass button bypassed (red)
    - Bypass button bypassed glow

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/LookAndFeel/ReOrderLookAndFeel.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T13.1 - Power Button Size (20×20px)
//==============================================================================
TEST_CASE("T13.1 PowerButton_Size", "[ui][button]")
{
    auto size = ReOrderLookAndFeel::getPowerButtonSize();
    REQUIRE(size == Approx(20.0f));
}

//==============================================================================
// T13.2 - Power Button Off Border Color (#444)
//==============================================================================
TEST_CASE("T13.2 PowerButton_Off_BorderColor", "[ui][button]")
{
    juce::Colour borderColor = ReOrderLookAndFeel::getPowerButtonOffBorderColor();
    REQUIRE(borderColor == juce::Colour(0xff444444));
}

//==============================================================================
// T13.3 - Power Button On Border Color (accent)
//==============================================================================
TEST_CASE("T13.3 PowerButton_On_BorderColor", "[ui][button]")
{
    juce::Colour testAccent = ReOrderColors::accentDelay;
    juce::Colour borderColor = ReOrderLookAndFeel::getPowerButtonOnBorderColor(testAccent);
    REQUIRE(borderColor == testAccent);
}

//==============================================================================
// T13.4 - Power Button Off Fill (transparent)
//==============================================================================
TEST_CASE("T13.4 PowerButton_Off_FillTransparent", "[ui][button]")
{
    juce::Colour fillColor = ReOrderLookAndFeel::getPowerButtonOffFillColor();
    REQUIRE(fillColor.isTransparent());
}

//==============================================================================
// T13.5 - Power Button On Fill (20% accent)
//==============================================================================
TEST_CASE("T13.5 PowerButton_On_FillTinted", "[ui][button]")
{
    juce::Colour testAccent = ReOrderColors::accentDelay;
    juce::Colour fillColor = ReOrderLookAndFeel::getPowerButtonOnFillColor(testAccent);

    // Should be same hue as accent but with ~20% alpha
    REQUIRE(fillColor.getHue() == Approx(testAccent.getHue()).margin(0.01f));
    REQUIRE(fillColor.getFloatAlpha() == Approx(0.2f).margin(0.05f));
}

//==============================================================================
// T13.6 - Toggle Button Off Background (#1a1a1a)
//==============================================================================
TEST_CASE("T13.6 ToggleButton_Off_Background", "[ui][button]")
{
    juce::Colour bgColor = ReOrderLookAndFeel::getToggleButtonOffBackground();
    REQUIRE(bgColor == juce::Colour(0xff1a1a1a));
}

//==============================================================================
// T13.7 - Toggle Button Off Text Color (#666)
//==============================================================================
TEST_CASE("T13.7 ToggleButton_Off_TextColor", "[ui][button]")
{
    juce::Colour textColor = ReOrderLookAndFeel::getToggleButtonOffTextColor();
    REQUIRE(textColor == juce::Colour(0xff666666));
}

//==============================================================================
// T13.8 - Toggle Button On Gradient Fill
//==============================================================================
TEST_CASE("T13.8 ToggleButton_On_GradientFill", "[ui][button]")
{
    bool hasGradient = ReOrderLookAndFeel::toggleButtonOnHasGradient();
    REQUIRE(hasGradient == true);
}

//==============================================================================
// T13.9 - Toggle Button On Glow
//==============================================================================
TEST_CASE("T13.9 ToggleButton_On_Glow", "[ui][button]")
{
    float glowAmount = ReOrderLookAndFeel::getToggleButtonOnGlowRadius();
    REQUIRE(glowAmount > 0.0f);
}

//==============================================================================
// T13.10 - Toggle Button Border Radius (4-6px)
//==============================================================================
TEST_CASE("T13.10 ToggleButton_BorderRadius", "[ui][button]")
{
    float radius = ReOrderLookAndFeel::getToggleButtonBorderRadius();
    REQUIRE(radius >= 4.0f);
    REQUIRE(radius <= 6.0f);
}

//==============================================================================
// T13.11 - Bypass Button Active (gray)
//==============================================================================
TEST_CASE("T13.11 BypassButton_Active_GrayColor", "[ui][button]")
{
    juce::Colour activeColor = ReOrderLookAndFeel::getBypassButtonActiveColor();
    // Should be a gray color
    REQUIRE(activeColor.getRed() == activeColor.getGreen());
    REQUIRE(activeColor.getGreen() == activeColor.getBlue());
}

//==============================================================================
// T13.12 - Bypass Button Bypassed (red)
//==============================================================================
TEST_CASE("T13.12 BypassButton_Bypassed_RedColor", "[ui][button]")
{
    juce::Colour bypassedColor = ReOrderLookAndFeel::getBypassButtonBypassedColor();
    REQUIRE(bypassedColor == ReOrderColors::statusError);  // #ff3366
}

//==============================================================================
// T13.13 - Bypass Button Bypassed Glow
//==============================================================================
TEST_CASE("T13.13 BypassButton_Bypassed_Glow", "[ui][button]")
{
    float glowRadius = ReOrderLookAndFeel::getBypassButtonBypassedGlowRadius();
    REQUIRE(glowRadius > 0.0f);
}
