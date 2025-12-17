#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/VoxProLookAndFeel.h"
#include "UI/LevelMeter.h"
#include "UI/CorrelationMeter.h"

/**
 * @brief HaasAlignDelayEditor - Main Editor with Auto Phase Correction UI
 *
 * Plugin editor providing the user interface for the Haas Align Delay.
 * Features scalable UI with preset sizes and intelligent auto phase feedback.
 */
class HaasAlignDelayEditor : public juce::AudioProcessorEditor,
                              private juce::Timer
{
public:
    explicit HaasAlignDelayEditor(HaasAlignDelayProcessor&);
    ~HaasAlignDelayEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCorrectionIndicator(juce::Graphics& g, juce::Rectangle<float> bounds, float scale);
    void setScalePreset(int presetIndex);

    HaasAlignDelayProcessor& processorRef;
    UI::VoxProLookAndFeel voxLookAndFeel;
    juce::Image logoImage;

    // Fixed scale presets (like FabFilter Pro-Q)
    static constexpr int baseWidth = 580;
    static constexpr int baseHeight = 380;
    static constexpr float scalePresets[] = { 0.5f, 0.75f, 1.0f, 1.5f };
    static constexpr int numScalePresets = 4;
    int currentScalePreset = 2;  // Default to 100%

    // Scale picker button
    juce::TextButton scaleButton;

    // Sliders
    juce::Slider delayLeftSlider;
    juce::Slider delayRightSlider;
    juce::Slider widthSlider;
    juce::Slider mixSlider;

    // Buttons
    juce::TextButton phaseLeftButton;
    juce::TextButton phaseRightButton;
    juce::TextButton bypassButton;
    juce::TextButton autoPhaseButton;

    // Phase Safety selector (Relaxed/Balanced/Strict)
    juce::ComboBox phaseSafetySelector;

    // Meters
    UI::LevelMeter inputMeter;
    UI::LevelMeter outputMeter;
    UI::CorrelationMeter correlationMeter;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phaseRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoPhaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> phaseSafetyAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HaasAlignDelayEditor)
};
