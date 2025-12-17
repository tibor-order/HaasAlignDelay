/*
  ==============================================================================

    OutputModuleTest.cpp
    Tests for Output Module Component (Iteration 18)

    Tests verify:
    - Accent color (green #00ff88)
    - Gain knob exists
    - Gain range (-12 to +12 dB)
    - Mix slider exists
    - Mix range (0-100%)
    - Output icon exists
    - Gain value format ("+0.0dB")
    - Mix value format ("100%")
    - Power button bypasses output
    - Parameter attachment

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/OutputModule.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T18.1 - Accent Color (green #00ff88)
//==============================================================================
TEST_CASE("T18.1 OutputModule_AccentColor", "[ui][outputmodule]")
{
    OutputModule module;
    REQUIRE(module.getAccentColor() == ReOrderColors::accentOutput);  // #00ff88
}

//==============================================================================
// T18.2 - Has Gain Knob
//==============================================================================
TEST_CASE("T18.2 OutputModule_HasGainKnob", "[ui][outputmodule]")
{
    OutputModule module;
    auto* gainKnob = module.getGainKnob();
    REQUIRE(gainKnob != nullptr);
}

//==============================================================================
// T18.3 - Gain Range (-12 to +12 dB)
//==============================================================================
TEST_CASE("T18.3 OutputModule_GainRange", "[ui][outputmodule]")
{
    OutputModule module;
    auto* gainKnob = module.getGainKnob();

    REQUIRE(gainKnob->getMinimum() == Approx(-12.0));
    REQUIRE(gainKnob->getMaximum() == Approx(12.0));
}

//==============================================================================
// T18.4 - Has Mix Slider
//==============================================================================
TEST_CASE("T18.4 OutputModule_HasMixSlider", "[ui][outputmodule]")
{
    OutputModule module;
    auto* mixSlider = module.getMixSlider();
    REQUIRE(mixSlider != nullptr);
}

//==============================================================================
// T18.5 - Mix Range (0-100%)
//==============================================================================
TEST_CASE("T18.5 OutputModule_MixRange", "[ui][outputmodule]")
{
    OutputModule module;
    auto* mixSlider = module.getMixSlider();

    REQUIRE(mixSlider->getMinimum() == Approx(0.0));
    REQUIRE(mixSlider->getMaximum() == Approx(100.0));
}

//==============================================================================
// T18.6 - Has Icon
//==============================================================================
TEST_CASE("T18.6 OutputModule_HasIcon", "[ui][outputmodule]")
{
    OutputModule module;
    REQUIRE(module.hasIcon() == true);
}

//==============================================================================
// T18.7 - Gain Value Format ("+0.0dB")
//==============================================================================
TEST_CASE("T18.7 OutputModule_GainValueFormat", "[ui][outputmodule]")
{
    OutputModule module;

    // Test positive gain
    module.getGainKnob()->setValue(3.5, juce::dontSendNotification);
    juce::String displayText = module.getGainValueText();
    REQUIRE(displayText.contains("+"));
    REQUIRE(displayText.contains("3.5"));
    REQUIRE(displayText.containsIgnoreCase("dB"));

    // Test negative gain
    module.getGainKnob()->setValue(-6.0, juce::dontSendNotification);
    displayText = module.getGainValueText();
    REQUIRE(displayText.contains("-"));
    REQUIRE(displayText.contains("6.0"));
    REQUIRE(displayText.containsIgnoreCase("dB"));

    // Test zero gain
    module.getGainKnob()->setValue(0.0, juce::dontSendNotification);
    displayText = module.getGainValueText();
    REQUIRE(displayText.contains("0.0"));
    REQUIRE(displayText.containsIgnoreCase("dB"));
}

//==============================================================================
// T18.8 - Mix Value Format ("100%")
//==============================================================================
TEST_CASE("T18.8 OutputModule_MixValueFormat", "[ui][outputmodule]")
{
    OutputModule module;

    module.getMixSlider()->setValue(75.0, juce::dontSendNotification);
    juce::String displayText = module.getMixValueText();

    REQUIRE(displayText.contains("75"));
    REQUIRE(displayText.contains("%"));
}

//==============================================================================
// T18.9 - Power Button Bypasses Output
//==============================================================================
TEST_CASE("T18.9 OutputModule_PowerButton_Bypass", "[ui][outputmodule]")
{
    OutputModule module;

    // Initially active
    REQUIRE(module.getActive() == true);

    // Click power button (toggle off)
    module.getPowerButton()->setToggleState(false, juce::sendNotification);

    // Should be inactive
    REQUIRE(module.getActive() == false);
}

//==============================================================================
// T18.10 - Parameter Attachment Ready
//==============================================================================
TEST_CASE("T18.10 OutputModule_ParameterAttachment", "[ui][outputmodule]")
{
    OutputModule module;

    // Verify controls are ready for attachment
    auto* gainKnob = module.getGainKnob();
    auto* mixSlider = module.getMixSlider();

    REQUIRE(gainKnob->isEnabled());
    REQUIRE(mixSlider->isEnabled());

    // Verify the module exposes parameter IDs
    REQUIRE(OutputModule::getGainParamID() == "outputGain");
    REQUIRE(OutputModule::getMixParamID() == "mix");
    REQUIRE(OutputModule::getBypassParamID() == "outputBypass");
}
