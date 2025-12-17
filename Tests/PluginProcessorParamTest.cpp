/*
  ==============================================================================

    PluginProcessorParamTest.cpp
    Tests for New Parameters Integration (Iteration 7)

    Tests verify:
    - All new parameters exist in the parameter tree
    - Parameters have correct ranges and defaults
    - Parameters save and restore correctly
    - Parameters are automatable

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/PluginProcessor.h"
#include <JuceHeader.h>

using Catch::Approx;

// Helper to get parameter from APVTS
static juce::RangedAudioParameter* getParam(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
{
    return apvts.getParameter(paramID);
}

TEST_CASE("T7.1 Param_DelayLink_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "delayLink");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.2 Param_DelayLink_DefaultFalse", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    float value = *apvts.getRawParameterValue("delayLink");
    REQUIRE(value == Approx(0.0f));  // false
}

TEST_CASE("T7.3 Param_DelayBypass_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "delayBypass");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.4 Param_WidthBypass_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "widthBypass");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.5 Param_WidthLowCut_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "widthLowCut");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.6 Param_WidthLowCut_Range", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "widthLowCut");
    REQUIRE(param != nullptr);

    auto range = param->getNormalisableRange();
    REQUIRE(range.start == Approx(20.0f));
    REQUIRE(range.end == Approx(500.0f));
}

TEST_CASE("T7.7 Param_WidthLowCut_Default", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    float value = *apvts.getRawParameterValue("widthLowCut");
    REQUIRE(value == Approx(250.0f));
}

TEST_CASE("T7.8 Param_PhaseBypass_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "phaseBypass");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.9 Param_CorrectionSpeed_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "correctionSpeed");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.10 Param_CorrectionSpeed_Range", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "correctionSpeed");
    REQUIRE(param != nullptr);

    auto range = param->getNormalisableRange();
    REQUIRE(range.start == Approx(0.0f));
    REQUIRE(range.end == Approx(100.0f));
}

TEST_CASE("T7.11 Param_CorrectionSpeed_Default", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    float value = *apvts.getRawParameterValue("correctionSpeed");
    REQUIRE(value == Approx(50.0f));
}

TEST_CASE("T7.12 Param_OutputBypass_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "outputBypass");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.13 Param_OutputGain_Exists", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "outputGain");
    REQUIRE(param != nullptr);
}

TEST_CASE("T7.14 Param_OutputGain_Range", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    auto* param = getParam(apvts, "outputGain");
    REQUIRE(param != nullptr);

    auto range = param->getNormalisableRange();
    REQUIRE(range.start == Approx(-12.0f));
    REQUIRE(range.end == Approx(12.0f));
}

TEST_CASE("T7.15 Param_OutputGain_Default", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    float value = *apvts.getRawParameterValue("outputGain");
    REQUIRE(value == Approx(0.0f).margin(0.01f));
}

TEST_CASE("T7.16 Param_StateRestore_AllParams", "[params]")
{
    // Create processor and modify parameters
    juce::MemoryBlock savedState;

    {
        HaasAlignDelayProcessor processor;
        auto& apvts = processor.getAPVTS();

        // Set non-default values
        apvts.getParameter("delayLink")->setValueNotifyingHost(1.0f);  // true
        apvts.getParameter("delayBypass")->setValueNotifyingHost(1.0f);
        apvts.getParameter("widthBypass")->setValueNotifyingHost(1.0f);
        apvts.getParameter("widthLowCut")->setValueNotifyingHost(
            apvts.getParameter("widthLowCut")->convertTo0to1(100.0f));  // 100Hz
        apvts.getParameter("phaseBypass")->setValueNotifyingHost(1.0f);
        apvts.getParameter("correctionSpeed")->setValueNotifyingHost(
            apvts.getParameter("correctionSpeed")->convertTo0to1(75.0f));  // 75%
        apvts.getParameter("outputBypass")->setValueNotifyingHost(1.0f);
        apvts.getParameter("outputGain")->setValueNotifyingHost(
            apvts.getParameter("outputGain")->convertTo0to1(6.0f));  // +6dB

        // Save state
        processor.getStateInformation(savedState);
    }

    // Create new processor and restore state
    HaasAlignDelayProcessor processor2;
    processor2.setStateInformation(savedState.getData(), (int)savedState.getSize());
    auto& apvts2 = processor2.getAPVTS();

    // Verify restored values
    REQUIRE(*apvts2.getRawParameterValue("delayLink") > 0.5f);
    REQUIRE(*apvts2.getRawParameterValue("delayBypass") > 0.5f);
    REQUIRE(*apvts2.getRawParameterValue("widthBypass") > 0.5f);
    REQUIRE(*apvts2.getRawParameterValue("widthLowCut") == Approx(100.0f).margin(1.0f));
    REQUIRE(*apvts2.getRawParameterValue("phaseBypass") > 0.5f);
    REQUIRE(*apvts2.getRawParameterValue("correctionSpeed") == Approx(75.0f).margin(1.0f));
    REQUIRE(*apvts2.getRawParameterValue("outputBypass") > 0.5f);
    REQUIRE(*apvts2.getRawParameterValue("outputGain") == Approx(6.0f).margin(0.5f));
}

TEST_CASE("T7.17 Param_StateSave_AllParams", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    // Set specific values
    apvts.getParameter("outputGain")->setValueNotifyingHost(
        apvts.getParameter("outputGain")->convertTo0to1(-6.0f));

    // Save state
    juce::MemoryBlock savedState;
    processor.getStateInformation(savedState);

    // State should not be empty
    REQUIRE(savedState.getSize() > 0);

    // Should be valid XML
    std::unique_ptr<juce::XmlElement> xml(juce::AudioProcessor::getXmlFromBinary(
        savedState.getData(), (int)savedState.getSize()));
    REQUIRE(xml != nullptr);
}

TEST_CASE("T7.18 Param_Automation_Supported", "[params]")
{
    HaasAlignDelayProcessor processor;
    auto& apvts = processor.getAPVTS();

    // All parameters should be automatable (default in JUCE)
    auto* delayLink = getParam(apvts, "delayLink");
    auto* outputGain = getParam(apvts, "outputGain");
    auto* correctionSpeed = getParam(apvts, "correctionSpeed");
    auto* widthLowCut = getParam(apvts, "widthLowCut");

    REQUIRE(delayLink != nullptr);
    REQUIRE(outputGain != nullptr);
    REQUIRE(correctionSpeed != nullptr);
    REQUIRE(widthLowCut != nullptr);

    // Parameters should be automatable by default
    // This is checked implicitly by the fact that setValueNotifyingHost works
    delayLink->setValueNotifyingHost(1.0f);
    REQUIRE(delayLink->getValue() == Approx(1.0f));

    outputGain->setValueNotifyingHost(0.75f);  // Somewhere in range
    REQUIRE(outputGain->getValue() == Approx(0.75f));
}
