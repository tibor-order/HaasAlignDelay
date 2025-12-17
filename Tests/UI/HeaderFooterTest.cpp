/*
  ==============================================================================

    HeaderFooterTest.cpp
    Tests for Header and Footer Components (Iteration 19)

    Tests verify:
    Header:
    - Height (48px)
    - Background gradient (#222 to #1a1a1a)
    - Has logo
    - Has preset selector
    - Has bypass button
    - Bypass button red when bypassed
    - Preset selector centered

    Footer:
    - Height (40px)
    - Background color (#0d0d0d)
    - "REORDER" in #444
    - "AUDIO" in cyan
    - Text centered

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/HeaderComponent.h"
#include "../Source/UI/FooterComponent.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T19.1 - Header Height (48px)
//==============================================================================
TEST_CASE("T19.1 Header_Height", "[ui][headerfooter]")
{
    float height = HeaderComponent::getDefaultHeight();
    REQUIRE(height == Approx(48.0f));
}

//==============================================================================
// T19.2 - Header Background Gradient
//==============================================================================
TEST_CASE("T19.2 Header_BackgroundGradient", "[ui][headerfooter]")
{
    auto [topColor, bottomColor] = HeaderComponent::getBackgroundGradientColors();

    // Top should be #222222
    REQUIRE(topColor == juce::Colour(0xff222222));
    // Bottom should be #1a1a1a
    REQUIRE(bottomColor == ReOrderColors::bgDark);
}

//==============================================================================
// T19.3 - Header Has Logo
//==============================================================================
TEST_CASE("T19.3 Header_HasLogo", "[ui][headerfooter]")
{
    HeaderComponent header;
    REQUIRE(header.hasLogo() == true);
}

//==============================================================================
// T19.4 - Header Has Preset Selector
//==============================================================================
TEST_CASE("T19.4 Header_HasPresetSelector", "[ui][headerfooter]")
{
    HeaderComponent header;
    REQUIRE(header.hasPresetSelector() == true);
}

//==============================================================================
// T19.5 - Header Has Bypass Button
//==============================================================================
TEST_CASE("T19.5 Header_HasBypassButton", "[ui][headerfooter]")
{
    HeaderComponent header;
    auto* bypassButton = header.getBypassButton();
    REQUIRE(bypassButton != nullptr);
}

//==============================================================================
// T19.6 - Header Bypass Button Red When Bypassed
//==============================================================================
TEST_CASE("T19.6 Header_BypassButton_RedWhenBypassed", "[ui][headerfooter]")
{
    HeaderComponent header;

    // When bypassed, the color should be red
    juce::Colour bypassedColor = header.getBypassButtonColor(true);
    REQUIRE(bypassedColor == ReOrderColors::statusError);  // #ff3366

    // When active (not bypassed), should be gray
    juce::Colour activeColor = header.getBypassButtonColor(false);
    REQUIRE(activeColor == juce::Colour(0xff555555));
}

//==============================================================================
// T19.7 - Header Preset Selector Centered
//==============================================================================
TEST_CASE("T19.7 Header_PresetSelector_Centered", "[ui][headerfooter]")
{
    HeaderComponent header;
    header.setSize(800, 48);

    auto presetBounds = header.getPresetSelectorBounds();
    int headerCenterX = header.getWidth() / 2;
    int presetCenterX = presetBounds.getCentreX();

    // Preset selector should be roughly centered (within 10px)
    REQUIRE(std::abs(headerCenterX - presetCenterX) < 10);
}

//==============================================================================
// T19.8 - Footer Height (40px)
//==============================================================================
TEST_CASE("T19.8 Footer_Height", "[ui][headerfooter]")
{
    float height = FooterComponent::getDefaultHeight();
    REQUIRE(height == Approx(40.0f));
}

//==============================================================================
// T19.9 - Footer Background Color (#0d0d0d)
//==============================================================================
TEST_CASE("T19.9 Footer_BackgroundColor", "[ui][headerfooter]")
{
    juce::Colour bgColor = FooterComponent::getBackgroundColor();
    REQUIRE(bgColor == ReOrderColors::bgDarkest);  // #0d0d0d
}

//==============================================================================
// T19.10 - Footer "REORDER" Color (#444)
//==============================================================================
TEST_CASE("T19.10 Footer_ReorderColor", "[ui][headerfooter]")
{
    juce::Colour reorderColor = FooterComponent::getReorderTextColor();
    REQUIRE(reorderColor == juce::Colour(0xff444444));
}

//==============================================================================
// T19.11 - Footer "AUDIO" Color (cyan)
//==============================================================================
TEST_CASE("T19.11 Footer_AudioColor", "[ui][headerfooter]")
{
    juce::Colour audioColor = FooterComponent::getAudioTextColor();
    REQUIRE(audioColor == ReOrderColors::accentWidth);  // Cyan #00d4ff
}

//==============================================================================
// T19.12 - Footer Text Centered
//==============================================================================
TEST_CASE("T19.12 Footer_TextCentered", "[ui][headerfooter]")
{
    FooterComponent footer;
    footer.setSize(800, 40);

    auto textBounds = footer.getBrandingTextBounds();
    int footerCenterX = footer.getWidth() / 2;
    int textCenterX = textBounds.getCentreX();

    // Text should be centered (within 5px)
    REQUIRE(std::abs(footerCenterX - textCenterX) < 5);
}
