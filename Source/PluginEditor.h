#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/NuroLookAndFeel.h"
#include "UI/NuroMeter.h"

/**
 * @brief HaasAlignDelayEditor - Nuro Audio Style UI
 *
 * Premium dark theme plugin editor matching Waves X-Vox Pro aesthetics.
 * Features glowing cyan knobs, LED-style meters, and neon accents.
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
    void drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& title);
    void drawValueDisplay(juce::Graphics& g, juce::Rectangle<float> bounds,
                          const juce::String& value, const juce::String& subValue = {});
    void drawCorrectionIndicator(juce::Graphics& g, juce::Rectangle<float> bounds);
    void setScalePreset(int presetIndex);

    HaasAlignDelayProcessor& processorRef;
    UI::NuroLookAndFeel nuroLookAndFeel;
    juce::Image logoImage;

    // Fixed scale presets (like FabFilter Pro-Q)
    static constexpr int baseWidth = 650;
    static constexpr int baseHeight = 420;
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

    // Meters - using new Nuro style
    UI::NuroLevelMeter inputMeter;
    UI::NuroLevelMeter outputMeter;
    UI::NuroCorrelationMeter correlationMeter;

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
