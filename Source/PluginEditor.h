#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/NuroLookAndFeel.h"
#include "UI/NuroMeter.h"
#include "UI/ResizeTriangle.h"

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
    float getScaleFactor() const;

    HaasAlignDelayProcessor& processorRef;
    UI::NuroLookAndFeel nuroLookAndFeel;
    juce::Image logoImage;

    // Size presets (16:10 aspect ratio like Waves plugins)
    struct SizePreset { int width; int height; const char* name; };
    static constexpr SizePreset sizePresets[] = {
        { 800, 500, "Small" },
        { 1000, 625, "Medium" },
        { 1200, 750, "Large" },
        { 1400, 875, "Extra Large" }
    };
    static constexpr int numSizePresets = 4;
    int currentSizePreset = 1;  // Default to Medium (1000x625)

    // Corner resize triangle with menu
    UI::ResizeTriangle resizeTriangle;

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
