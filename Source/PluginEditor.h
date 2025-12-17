#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/NuroLookAndFeel.h"
#include "UI/NuroMeter.h"
#include "UI/ResizeTriangle.h"

/**
 * @brief HaasAlignDelayEditor - HaasFX Pro Style UI
 *
 * Modern horizontal modular layout with:
 * - Input/Output level meters
 * - Delay module (pink)
 * - Width module (cyan)
 * - Auto Phase module (orange) - THE STAR FEATURE
 * - Output module (green)
 * - Proportional scaling with resize triangle
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
    void setScalePreset(int presetIndex);
    float getScale() const { return currentScale; }

    // Drawing helpers
    void drawHeader(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawFooter(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawDelayModule(juce::Graphics& g, juce::Rectangle<int> bounds, bool isOn);
    void drawWidthModule(juce::Graphics& g, juce::Rectangle<int> bounds, bool isOn);
    void drawAutoPhaseModule(juce::Graphics& g, juce::Rectangle<int> bounds,
                              bool isOn, bool isCorrecting);
    void drawOutputModule(juce::Graphics& g, juce::Rectangle<int> bounds, bool isOn);
    void drawInputMeters(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawOutputMeters(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSegmentedMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
                             float level, const juce::String& label);
    void drawKnob(juce::Graphics& g, juce::Rectangle<int> bounds,
                  const juce::String& label, const juce::String& value,
                  juce::Colour accentColor, float normalizedValue, bool isActive);
    void drawVerticalSlider(juce::Graphics& g, juce::Rectangle<int> bounds,
                            const juce::String& label, const juce::String& value,
                            juce::Colour accentColor, float normalizedValue, bool isActive);
    void drawCorrelationMeter(juce::Graphics& g, juce::Rectangle<int> bounds,
                               float correlation, float threshold, bool isActive);
    void drawPowerButton(juce::Graphics& g, juce::Rectangle<int> bounds,
                         juce::Colour accentColor, bool isOn);
    void drawWaveIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                      juce::Colour color, bool isActive);
    void drawExpandIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                        juce::Colour color, bool isActive);
    void drawPhaseIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                       juce::Colour color, bool isActive, bool isCorrecting);
    void drawSpeakerIcon(juce::Graphics& g, juce::Rectangle<int> bounds,
                         juce::Colour color, bool isActive);

    HaasAlignDelayProcessor& processorRef;
    UI::NuroLookAndFeel nuroLookAndFeel;

    // Module bounds (for click detection)
    juce::Rectangle<int> delayModuleBounds;
    juce::Rectangle<int> widthModuleBounds;
    juce::Rectangle<int> autoPhaseModuleBounds;
    juce::Rectangle<int> outputModuleBounds;

    // Power button bounds
    juce::Rectangle<int> delayPowerBounds;
    juce::Rectangle<int> widthPowerBounds;
    juce::Rectangle<int> autoPhasePowerBounds;
    juce::Rectangle<int> outputPowerBounds;

    // Sliders
    juce::Slider delayLeftSlider;
    juce::Slider delayRightSlider;
    juce::Slider widthSlider;
    juce::Slider lowCutSlider;
    juce::Slider thresholdSlider;
    juce::Slider speedSlider;
    juce::Slider outputGainSlider;
    juce::Slider mixSlider;

    // Buttons
    juce::TextButton delayPowerButton;
    juce::TextButton widthPowerButton;
    juce::TextButton autoPhasePowerButton;
    juce::TextButton outputPowerButton;
    juce::TextButton delayLinkButton;
    juce::TextButton bypassButton;
    juce::TextButton autoFixButton;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayLeftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayRightAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> delayPowerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> widthPowerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoPhasePowerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> outputPowerAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> delayLinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Animation
    float waveAnimPhase = 0.0f;
    float correctionGlowIntensity = 0.0f;

    // Metering
    float inputLevelL = 0.0f;
    float inputLevelR = 0.0f;
    float outputLevelL = 0.0f;
    float outputLevelR = 0.0f;

    // Resize triangle and scaling
    UI::ResizeTriangle resizeTriangle;

    // Base size (Medium = 1000x625)
    static constexpr int baseWidth = 1000;
    static constexpr int baseHeight = 625;

    // Size presets (proportional scaling)
    struct SizePreset {
        const char* name;
        int width;
        int height;
        float scale;
    };
    static constexpr SizePreset sizePresets[] = {
        {"Small",       800,  500, 0.8f},
        {"Medium",     1000,  625, 1.0f},
        {"Large",      1200,  750, 1.2f},
        {"Extra Large", 1400, 875, 1.4f}
    };
    static constexpr int numPresets = 4;
    int currentPresetIndex = 1;  // Default to Medium
    float currentScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HaasAlignDelayEditor)
};
