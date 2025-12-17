/*
  ==============================================================================

    PluginEditor.h
    HAAS FX Pro - Main Editor with ReOrder Audio Design System

    Layout:
    ┌──────────────────────────────────────────────────────────────┐
    │ HEADER (48px)                                                │
    ├────┬─────────┬─────────┬─────────┬─────────┬────────────────┤
    │ IN │  DELAY  │  WIDTH  │  PHASE  │ OUTPUT  │      OUT       │
    │ 50 │  flex   │  flex   │  flex   │  flex   │      50        │
    ├────┴─────────┴─────────┴─────────┴─────────┴────────────────┤
    │ FOOTER (40px)                                                │
    └──────────────────────────────────────────────────────────────┘

    Size: 1000 x 625px (default)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/ReOrderColors.h"
#include "UI/HeaderComponent.h"
#include "UI/FooterComponent.h"
#include "UI/MeterStrip.h"
#include "UI/DelayModule.h"
#include "UI/WidthModule.h"
#include "UI/PhaseModule.h"
#include "UI/OutputModule.h"
#include "UI/AnimationSystem.h"
#include "Presets/PresetManager.h"

/**
 * @brief HaasAlignDelayEditor - Main Editor with ReOrder Audio Design
 */
class HaasAlignDelayEditor : public juce::AudioProcessorEditor,
                              private juce::Timer
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr int DEFAULT_WIDTH = 1000;
    static constexpr int DEFAULT_HEIGHT = 625;
    static constexpr int HEADER_HEIGHT = 48;
    static constexpr int FOOTER_HEIGHT = 40;
    static constexpr int METER_STRIP_WIDTH = 50;
    static constexpr int MODULE_SPACING = 15;

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    explicit HaasAlignDelayEditor(HaasAlignDelayProcessor&);
    ~HaasAlignDelayEditor() override;

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Timer Callback
    //==============================================================================

    void timerCallback() override;

    //==============================================================================
    // Members
    //==============================================================================

    HaasAlignDelayProcessor& processorRef;

    // Preset Manager
    PresetManager presetManager;

    // Animation Controller
    AnimationSystem::AnimationController animationController;

    // Main Components
    std::unique_ptr<HeaderComponent> header;
    std::unique_ptr<FooterComponent> footer;
    std::unique_ptr<MeterStrip> inputMeterStrip;
    std::unique_ptr<MeterStrip> outputMeterStrip;

    // Module Components
    std::unique_ptr<DelayModule> delayModule;
    std::unique_ptr<WidthModule> widthModule;
    std::unique_ptr<PhaseModule> phaseModule;
    std::unique_ptr<OutputModule> outputModule;

    // Parameter Attachments - Delay Module
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> delayLinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> delayBypassAttachment;

    // Parameter Attachments - Width Module
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthLowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> widthBypassAttachment;

    // Parameter Attachments - Phase Module
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoPhaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> correctionSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseBypassAttachment;

    // Parameter Attachments - Output Module
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> outputBypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HaasAlignDelayEditor)
};
