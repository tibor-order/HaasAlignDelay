/*
  ==============================================================================

    OutputModule.h
    Output module panel with gain and mix controls

    Layout:
    [●] ──── OUTPUT ────
          [Output Icon]

          [Gain Knob]
            +0.0dB
             GAIN

      DRY/WET          100%
      [━━━━━━━━━━━━━━━━━●]

    Features:
    - Gain knob: -12 to +12 dB
    - Dry/wet horizontal slider: 0-100%
    - Green accent color (#00ff88)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModuleComponent.h"
#include "ModuleIcons.h"
#include "ReOrderColors.h"

/**
 * @brief Output module component with gain and mix controls
 */
class OutputModule : public ModuleComponent
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float GAIN_MIN = -12.0f;
    static constexpr float GAIN_MAX = 12.0f;
    static constexpr float GAIN_DEFAULT = 0.0f;

    static constexpr float MIX_MIN = 0.0f;
    static constexpr float MIX_MAX = 100.0f;
    static constexpr float MIX_DEFAULT = 100.0f;

    //==============================================================================
    // Parameter IDs
    //==============================================================================

    static juce::String getGainParamID() { return "outputGain"; }
    static juce::String getMixParamID() { return "mix"; }
    static juce::String getBypassParamID() { return "outputBypass"; }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    OutputModule()
        : ModuleComponent("OUTPUT", ReOrderColors::accentOutput)
    {
        // Create gain knob
        gainKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                   juce::Slider::NoTextBox);
        gainKnob->setRange(GAIN_MIN, GAIN_MAX, 0.1);
        gainKnob->setValue(GAIN_DEFAULT);
        gainKnob->setColour(juce::Slider::rotarySliderFillColourId, getAccentColor());
        gainKnob->onValueChange = [this]() { updateGainLabel(); };
        addAndMakeVisible(gainKnob.get());

        // Create mix slider
        mixSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                    juce::Slider::NoTextBox);
        mixSlider->setRange(MIX_MIN, MIX_MAX, 1.0);
        mixSlider->setValue(MIX_DEFAULT);
        mixSlider->onValueChange = [this]() { updateMixLabel(); };
        addAndMakeVisible(mixSlider.get());

        // Create labels
        gainValueLabel = std::make_unique<juce::Label>("gainValue", "+0.0dB");
        gainValueLabel->setJustificationType(juce::Justification::centred);
        gainValueLabel->setColour(juce::Label::textColourId, getAccentColor());
        addAndMakeVisible(gainValueLabel.get());

        gainLabel = std::make_unique<juce::Label>("gainLabel", "GAIN");
        gainLabel->setJustificationType(juce::Justification::centred);
        gainLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(gainLabel.get());

        mixLabel = std::make_unique<juce::Label>("mixLabel", "DRY/WET");
        mixLabel->setJustificationType(juce::Justification::left);
        mixLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(mixLabel.get());

        mixValueLabel = std::make_unique<juce::Label>("mixValue", "100%");
        mixValueLabel->setJustificationType(juce::Justification::right);
        mixValueLabel->setColour(juce::Label::textColourId, getAccentColor());
        addAndMakeVisible(mixValueLabel.get());
    }

    ~OutputModule() override = default;

    //==============================================================================
    // Control Accessors
    //==============================================================================

    juce::Slider* getGainKnob() { return gainKnob.get(); }
    juce::Slider* getMixSlider() { return mixSlider.get(); }

    //==============================================================================
    // Icon
    //==============================================================================

    bool hasIcon() const { return true; }

    //==============================================================================
    // Value Text
    //==============================================================================

    juce::String getGainValueText() const
    {
        double value = gainKnob->getValue();
        juce::String prefix = (value >= 0.0) ? "+" : "";
        return prefix + juce::String(value, 1) + "dB";
    }

    juce::String getMixValueText() const
    {
        return juce::String(static_cast<int>(mixSlider->getValue())) + "%";
    }

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        // Draw base module
        ModuleComponent::paint(g);

        // Draw icon in icon area
        auto iconBounds = getIconBounds().toFloat();
        ModuleIcons::drawIcon(g, ModuleIcons::IconType::Output, iconBounds, getActive());
    }

    void resized() override
    {
        ModuleComponent::resized();

        auto controlsBounds = getControlsBounds();
        int padding = 10;
        int centerX = controlsBounds.getCentreX();

        // Gain knob
        int knobSize = 70;
        int knobY = controlsBounds.getY() + padding;
        gainKnob->setBounds(centerX - knobSize / 2, knobY, knobSize, knobSize);

        // Gain value label
        int labelHeight = 20;
        int gainValueY = knobY + knobSize + 5;
        gainValueLabel->setBounds(centerX - 45, gainValueY, 90, labelHeight);

        // Gain label
        int gainLabelY = gainValueY + labelHeight;
        gainLabel->setBounds(centerX - 30, gainLabelY, 60, labelHeight);

        // Mix section
        int mixY = gainLabelY + labelHeight + 20;
        int sliderWidth = controlsBounds.getWidth() - padding * 4;

        // Mix labels
        mixLabel->setBounds(padding * 2, mixY, 60, labelHeight);
        mixValueLabel->setBounds(controlsBounds.getWidth() - padding * 2 - 50, mixY, 50, labelHeight);

        // Mix slider
        int sliderY = mixY + labelHeight + 5;
        mixSlider->setBounds(padding * 2, sliderY, sliderWidth, 20);
    }

private:
    void updateGainLabel()
    {
        gainValueLabel->setText(getGainValueText(), juce::dontSendNotification);
    }

    void updateMixLabel()
    {
        mixValueLabel->setText(getMixValueText(), juce::dontSendNotification);
    }

    //==============================================================================
    // Members
    //==============================================================================

    std::unique_ptr<juce::Slider> gainKnob;
    std::unique_ptr<juce::Slider> mixSlider;

    std::unique_ptr<juce::Label> gainValueLabel;
    std::unique_ptr<juce::Label> gainLabel;
    std::unique_ptr<juce::Label> mixLabel;
    std::unique_ptr<juce::Label> mixValueLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputModule)
};
