/*
  ==============================================================================

    WidthModuleTest.cpp
    Tests for Width Module Component (Iteration 16)

    Tests verify:
    - Accent color (cyan #00d4ff)
    - Width knob exists
    - Width range (0-200%)
    - Low cut slider exists
    - Low cut range (20-500Hz)
    - Phase L button exists
    - Phase R button exists
    - Width icon exists
    - Width value format ("150%")
    - Low cut value format ("250Hz")
    - Power button bypasses width
    - Parameter attachment

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/WidthModule.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T16.1 - Accent Color (cyan #00d4ff)
//==============================================================================
TEST_CASE("T16.1 WidthModule_AccentColor", "[ui][widthmodule]")
{
    WidthModule module;
    REQUIRE(module.getAccentColor() == ReOrderColors::accentWidth);  // #00d4ff
}

//==============================================================================
// T16.2 - Has Width Knob
//==============================================================================
TEST_CASE("T16.2 WidthModule_HasWidthKnob", "[ui][widthmodule]")
{
    WidthModule module;
    auto* widthKnob = module.getWidthKnob();
    REQUIRE(widthKnob != nullptr);
}

//==============================================================================
// T16.3 - Width Range (0-200%)
//==============================================================================
TEST_CASE("T16.3 WidthModule_WidthRange", "[ui][widthmodule]")
{
    WidthModule module;
    auto* widthKnob = module.getWidthKnob();

    REQUIRE(widthKnob->getMinimum() == Approx(0.0));
    REQUIRE(widthKnob->getMaximum() == Approx(200.0));
}

//==============================================================================
// T16.4 - Has Low Cut Slider
//==============================================================================
TEST_CASE("T16.4 WidthModule_HasLowCutSlider", "[ui][widthmodule]")
{
    WidthModule module;
    auto* lowCutSlider = module.getLowCutSlider();
    REQUIRE(lowCutSlider != nullptr);
}

//==============================================================================
// T16.5 - Low Cut Range (20-500Hz)
//==============================================================================
TEST_CASE("T16.5 WidthModule_LowCutRange", "[ui][widthmodule]")
{
    WidthModule module;
    auto* lowCutSlider = module.getLowCutSlider();

    REQUIRE(lowCutSlider->getMinimum() == Approx(20.0));
    REQUIRE(lowCutSlider->getMaximum() == Approx(500.0));
}

//==============================================================================
// T16.6 - Has Phase L Button
//==============================================================================
TEST_CASE("T16.6 WidthModule_HasPhaseLButton", "[ui][widthmodule]")
{
    WidthModule module;
    auto* phaseLButton = module.getPhaseLButton();
    REQUIRE(phaseLButton != nullptr);
}

//==============================================================================
// T16.7 - Has Phase R Button
//==============================================================================
TEST_CASE("T16.7 WidthModule_HasPhaseRButton", "[ui][widthmodule]")
{
    WidthModule module;
    auto* phaseRButton = module.getPhaseRButton();
    REQUIRE(phaseRButton != nullptr);
}

//==============================================================================
// T16.8 - Has Icon
//==============================================================================
TEST_CASE("T16.8 WidthModule_HasIcon", "[ui][widthmodule]")
{
    WidthModule module;
    REQUIRE(module.hasIcon() == true);
}

//==============================================================================
// T16.9 - Width Value Format ("150%")
//==============================================================================
TEST_CASE("T16.9 WidthModule_WidthValueFormat", "[ui][widthmodule]")
{
    WidthModule module;

    module.getWidthKnob()->setValue(150.0, juce::dontSendNotification);
    juce::String displayText = module.getWidthValueText();

    // Should be formatted like "150%"
    REQUIRE(displayText.contains("150"));
    REQUIRE(displayText.contains("%"));
}

//==============================================================================
// T16.10 - Low Cut Value Format ("250Hz")
//==============================================================================
TEST_CASE("T16.10 WidthModule_LowCutValueFormat", "[ui][widthmodule]")
{
    WidthModule module;

    module.getLowCutSlider()->setValue(250.0, juce::dontSendNotification);
    juce::String displayText = module.getLowCutValueText();

    // Should be formatted like "250Hz"
    REQUIRE(displayText.contains("250"));
    REQUIRE(displayText.containsIgnoreCase("Hz"));
}

//==============================================================================
// T16.11 - Power Button Bypasses Width
//==============================================================================
TEST_CASE("T16.11 WidthModule_PowerButton_Bypass", "[ui][widthmodule]")
{
    WidthModule module;

    // Initially active
    REQUIRE(module.getActive() == true);

    // Click power button (toggle off)
    module.getPowerButton()->setToggleState(false, juce::sendNotification);

    // Should be inactive
    REQUIRE(module.getActive() == false);
}

//==============================================================================
// T16.12 - Parameter Attachment Ready
//==============================================================================
TEST_CASE("T16.12 WidthModule_ParameterAttachment", "[ui][widthmodule]")
{
    WidthModule module;

    // Verify controls can be attached to parameters
    auto* widthKnob = module.getWidthKnob();
    auto* lowCutSlider = module.getLowCutSlider();

    REQUIRE(widthKnob->isEnabled());
    REQUIRE(lowCutSlider->isEnabled());

    // Verify the module exposes parameter IDs
    REQUIRE(WidthModule::getWidthParamID() == "width");
    REQUIRE(WidthModule::getLowCutParamID() == "widthLowCut");
    REQUIRE(WidthModule::getPhaseLParamID() == "phaseInvertL");
    REQUIRE(WidthModule::getPhaseRParamID() == "phaseInvertR");
}
