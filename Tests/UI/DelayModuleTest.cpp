/*
  ==============================================================================

    DelayModuleTest.cpp
    Tests for Delay Module Component (Iteration 15)

    Tests verify:
    - Accent color (pink #ff66aa)
    - Left delay slider exists
    - Right delay slider exists
    - Slider range (0-50ms)
    - LINK toggle button exists
    - HaasIcon exists
    - Unlinked sliders move independently
    - Linked sliders move together
    - Link maintains offset
    - Value display format ("0.0ms")
    - Power button bypasses delay
    - Parameter attachment

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/DelayModule.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T15.1 - Accent Color (pink #ff66aa)
//==============================================================================
TEST_CASE("T15.1 DelayModule_AccentColor", "[ui][delaymodule]")
{
    DelayModule module;
    REQUIRE(module.getAccentColor() == ReOrderColors::accentDelay);  // #ff66aa
}

//==============================================================================
// T15.2 - Has Left Slider
//==============================================================================
TEST_CASE("T15.2 DelayModule_HasLeftSlider", "[ui][delaymodule]")
{
    DelayModule module;
    auto* leftSlider = module.getLeftDelaySlider();
    REQUIRE(leftSlider != nullptr);
}

//==============================================================================
// T15.3 - Has Right Slider
//==============================================================================
TEST_CASE("T15.3 DelayModule_HasRightSlider", "[ui][delaymodule]")
{
    DelayModule module;
    auto* rightSlider = module.getRightDelaySlider();
    REQUIRE(rightSlider != nullptr);
}

//==============================================================================
// T15.4 - Slider Range (0-50ms)
//==============================================================================
TEST_CASE("T15.4 DelayModule_SliderRange", "[ui][delaymodule]")
{
    DelayModule module;
    auto* leftSlider = module.getLeftDelaySlider();
    auto* rightSlider = module.getRightDelaySlider();

    REQUIRE(leftSlider->getMinimum() == Approx(0.0));
    REQUIRE(leftSlider->getMaximum() == Approx(50.0));
    REQUIRE(rightSlider->getMinimum() == Approx(0.0));
    REQUIRE(rightSlider->getMaximum() == Approx(50.0));
}

//==============================================================================
// T15.5 - Has LINK Button
//==============================================================================
TEST_CASE("T15.5 DelayModule_HasLinkButton", "[ui][delaymodule]")
{
    DelayModule module;
    auto* linkButton = module.getLinkButton();
    REQUIRE(linkButton != nullptr);
}

//==============================================================================
// T15.6 - Has Icon
//==============================================================================
TEST_CASE("T15.6 DelayModule_HasIcon", "[ui][delaymodule]")
{
    DelayModule module;
    REQUIRE(module.hasIcon() == true);
}

//==============================================================================
// T15.7 - Link Off = Independent Movement
//==============================================================================
TEST_CASE("T15.7 DelayModule_LinkOff_Independent", "[ui][delaymodule]")
{
    DelayModule module;

    // Ensure link is off
    module.setLinkEnabled(false);

    // Set initial values
    module.getLeftDelaySlider()->setValue(10.0, juce::dontSendNotification);
    module.getRightDelaySlider()->setValue(20.0, juce::dontSendNotification);

    // Move left slider
    module.getLeftDelaySlider()->setValue(15.0, juce::sendNotification);

    // Right should be unchanged
    REQUIRE(module.getRightDelaySlider()->getValue() == Approx(20.0));
}

//==============================================================================
// T15.8 - Link On = Coupled Movement
//==============================================================================
TEST_CASE("T15.8 DelayModule_LinkOn_Coupled", "[ui][delaymodule]")
{
    DelayModule module;

    // Set initial values (same)
    module.getLeftDelaySlider()->setValue(10.0, juce::dontSendNotification);
    module.getRightDelaySlider()->setValue(10.0, juce::dontSendNotification);

    // Enable link
    module.setLinkEnabled(true);

    // Move left slider by 5ms (use sendNotificationSync for immediate callback)
    module.getLeftDelaySlider()->setValue(15.0, juce::sendNotificationSync);

    // Right should move by same amount
    REQUIRE(module.getRightDelaySlider()->getValue() == Approx(15.0));
}

//==============================================================================
// T15.9 - Link On = Maintains Offset
//==============================================================================
TEST_CASE("T15.9 DelayModule_LinkOn_MaintainsOffset", "[ui][delaymodule]")
{
    DelayModule module;

    // Set initial values with 10ms offset
    module.getLeftDelaySlider()->setValue(5.0, juce::dontSendNotification);
    module.getRightDelaySlider()->setValue(15.0, juce::dontSendNotification);

    // Enable link (captures offset)
    module.setLinkEnabled(true);

    // Move left slider by 5ms (use sendNotificationSync for immediate callback)
    module.getLeftDelaySlider()->setValue(10.0, juce::sendNotificationSync);

    // Right should maintain 10ms offset: 10 + 10 = 20
    REQUIRE(module.getRightDelaySlider()->getValue() == Approx(20.0));
}

//==============================================================================
// T15.10 - Value Display Format ("0.0ms")
//==============================================================================
TEST_CASE("T15.10 DelayModule_ValueDisplay_Format", "[ui][delaymodule]")
{
    DelayModule module;

    module.getLeftDelaySlider()->setValue(12.5, juce::dontSendNotification);
    juce::String displayText = module.getLeftValueText();

    // Should be formatted like "12.5ms"
    REQUIRE(displayText.contains("12.5"));
    REQUIRE(displayText.containsIgnoreCase("ms"));
}

//==============================================================================
// T15.11 - Power Button Bypasses Delay
//==============================================================================
TEST_CASE("T15.11 DelayModule_PowerButton_Bypass", "[ui][delaymodule]")
{
    DelayModule module;

    // Initially active
    REQUIRE(module.getActive() == true);

    // Click power button (toggle off)
    module.getPowerButton()->setToggleState(false, juce::sendNotification);

    // Should be inactive
    REQUIRE(module.getActive() == false);
}

//==============================================================================
// T15.12 - Parameter Attachment Ready
//==============================================================================
TEST_CASE("T15.12 DelayModule_ParameterAttachment", "[ui][delaymodule]")
{
    DelayModule module;

    // Verify sliders can be attached to parameters
    // (just checking they have valid ranges and are ready for attachment)
    auto* leftSlider = module.getLeftDelaySlider();
    auto* rightSlider = module.getRightDelaySlider();

    REQUIRE(leftSlider->isEnabled());
    REQUIRE(rightSlider->isEnabled());

    // Verify the module exposes parameter IDs
    REQUIRE(DelayModule::getLeftDelayParamID() == "delayLeft");
    REQUIRE(DelayModule::getRightDelayParamID() == "delayRight");
    REQUIRE(DelayModule::getLinkParamID() == "delayLink");
}
