/*
  ==============================================================================

    WidthModule.h
    Width module panel with low cut and phase buttons

    Layout:
    [●] ──── WIDTH ────
          [Width Icon]
         [Width Knob]
            150%
        STEREO WIDTH

      LOW CUT        250Hz
      [━━━━━●━━━━━━━━━━━]

        [⦿L]    [⦿R]
       PHASE L  PHASE R

    Features:
    - Width knob: 0-200%
    - Low cut slider: 20-500Hz horizontal
    - Phase invert buttons for L/R
    - Cyan accent color (#00d4ff)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModuleComponent.h"
#include "ModuleIcons.h"
#include "ReOrderColors.h"

/**
 * @brief Width module component with stereo width, low cut, and phase controls
 */
class WidthModule : public ModuleComponent
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float WIDTH_MIN = 0.0f;
    static constexpr float WIDTH_MAX = 200.0f;
    static constexpr float WIDTH_DEFAULT = 100.0f;

    static constexpr float LOWCUT_MIN = 20.0f;
    static constexpr float LOWCUT_MAX = 500.0f;
    static constexpr float LOWCUT_DEFAULT = 250.0f;

    //==============================================================================
    // Parameter IDs
    //==============================================================================

    static juce::String getWidthParamID() { return "width"; }
    static juce::String getLowCutParamID() { return "widthLowCut"; }
    static juce::String getPhaseLParamID() { return "phaseInvertL"; }
    static juce::String getPhaseRParamID() { return "phaseInvertR"; }
    static juce::String getBypassParamID() { return "widthBypass"; }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    WidthModule()
        : ModuleComponent("WIDTH", ReOrderColors::accentWidth)
    {
        // Create width knob
        widthKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                    juce::Slider::NoTextBox);
        widthKnob->setRange(WIDTH_MIN, WIDTH_MAX, 1.0);
        widthKnob->setValue(WIDTH_DEFAULT);
        widthKnob->setColour(juce::Slider::rotarySliderFillColourId, getAccentColor());
        addAndMakeVisible(widthKnob.get());

        // Create low cut slider
        lowCutSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                       juce::Slider::NoTextBox);
        lowCutSlider->setRange(LOWCUT_MIN, LOWCUT_MAX, 1.0);
        lowCutSlider->setValue(LOWCUT_DEFAULT);
        lowCutSlider->setSkewFactorFromMidPoint(100.0);  // Logarithmic feel
        addAndMakeVisible(lowCutSlider.get());

        // Create phase buttons
        phaseLButton = std::make_unique<juce::ToggleButton>("L");
        phaseLButton->setClickingTogglesState(true);
        addAndMakeVisible(phaseLButton.get());

        phaseRButton = std::make_unique<juce::ToggleButton>("R");
        phaseRButton->setClickingTogglesState(true);
        addAndMakeVisible(phaseRButton.get());

        // Create value labels
        widthValueLabel = std::make_unique<juce::Label>("widthValue", "100%");
        widthValueLabel->setJustificationType(juce::Justification::centred);
        widthValueLabel->setColour(juce::Label::textColourId, getAccentColor());
        addAndMakeVisible(widthValueLabel.get());

        widthLabel = std::make_unique<juce::Label>("widthLabel", "STEREO WIDTH");
        widthLabel->setJustificationType(juce::Justification::centred);
        widthLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(widthLabel.get());

        lowCutLabel = std::make_unique<juce::Label>("lowCutLabel", "LOW CUT");
        lowCutLabel->setJustificationType(juce::Justification::left);
        lowCutLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(lowCutLabel.get());

        lowCutValueLabel = std::make_unique<juce::Label>("lowCutValue", "250Hz");
        lowCutValueLabel->setJustificationType(juce::Justification::right);
        lowCutValueLabel->setColour(juce::Label::textColourId, getAccentColor());
        addAndMakeVisible(lowCutValueLabel.get());

        phaseLLabel = std::make_unique<juce::Label>("phaseLLabel", "PHASE L");
        phaseLLabel->setJustificationType(juce::Justification::centred);
        phaseLLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(phaseLLabel.get());

        phaseRLabel = std::make_unique<juce::Label>("phaseRLabel", "PHASE R");
        phaseRLabel->setJustificationType(juce::Justification::centred);
        phaseRLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(phaseRLabel.get());

        // Setup callbacks
        widthKnob->onValueChange = [this]() { updateWidthLabel(); };
        lowCutSlider->onValueChange = [this]() { updateLowCutLabel(); };
    }

    ~WidthModule() override = default;

    //==============================================================================
    // Control Accessors
    //==============================================================================

    juce::Slider* getWidthKnob() { return widthKnob.get(); }
    juce::Slider* getLowCutSlider() { return lowCutSlider.get(); }
    juce::Button* getPhaseLButton() { return phaseLButton.get(); }
    juce::Button* getPhaseRButton() { return phaseRButton.get(); }

    //==============================================================================
    // Icon
    //==============================================================================

    bool hasIcon() const { return true; }

    //==============================================================================
    // Value Text
    //==============================================================================

    juce::String getWidthValueText() const
    {
        return juce::String(static_cast<int>(widthKnob->getValue())) + "%";
    }

    juce::String getLowCutValueText() const
    {
        return juce::String(static_cast<int>(lowCutSlider->getValue())) + "Hz";
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
        ModuleIcons::drawIcon(g, ModuleIcons::IconType::Width, iconBounds, getActive());
    }

    void resized() override
    {
        ModuleComponent::resized();

        auto controlsBounds = getControlsBounds();
        int padding = 10;
        int centerX = controlsBounds.getCentreX();

        // Width knob
        int knobSize = 70;
        int knobY = controlsBounds.getY() + padding;
        widthKnob->setBounds(centerX - knobSize / 2, knobY, knobSize, knobSize);

        // Width value label
        int labelHeight = 20;
        int widthValueY = knobY + knobSize + 5;
        widthValueLabel->setBounds(centerX - 40, widthValueY, 80, labelHeight);

        // Width label
        int widthLabelY = widthValueY + labelHeight;
        widthLabel->setBounds(centerX - 60, widthLabelY, 120, labelHeight);

        // Low cut section
        int lowCutY = widthLabelY + labelHeight + 15;
        int sliderWidth = controlsBounds.getWidth() - padding * 4;

        // Low cut labels
        lowCutLabel->setBounds(padding * 2, lowCutY, 60, labelHeight);
        lowCutValueLabel->setBounds(controlsBounds.getWidth() - padding * 2 - 60, lowCutY, 60, labelHeight);

        // Low cut slider
        int sliderY = lowCutY + labelHeight + 5;
        lowCutSlider->setBounds(padding * 2, sliderY, sliderWidth, 20);

        // Phase buttons
        int buttonY = sliderY + 35;
        int buttonSize = 30;
        int buttonSpacing = 50;

        phaseLButton->setBounds(centerX - buttonSpacing - buttonSize / 2, buttonY, buttonSize, buttonSize);
        phaseRButton->setBounds(centerX + buttonSpacing - buttonSize / 2, buttonY, buttonSize, buttonSize);

        // Phase labels
        int phaseLabelY = buttonY + buttonSize + 2;
        phaseLLabel->setBounds(centerX - buttonSpacing - 35, phaseLabelY, 70, labelHeight);
        phaseRLabel->setBounds(centerX + buttonSpacing - 35, phaseLabelY, 70, labelHeight);
    }

private:
    void updateWidthLabel()
    {
        widthValueLabel->setText(getWidthValueText(), juce::dontSendNotification);
    }

    void updateLowCutLabel()
    {
        lowCutValueLabel->setText(getLowCutValueText(), juce::dontSendNotification);
    }

    //==============================================================================
    // Members
    //==============================================================================

    std::unique_ptr<juce::Slider> widthKnob;
    std::unique_ptr<juce::Slider> lowCutSlider;
    std::unique_ptr<juce::ToggleButton> phaseLButton;
    std::unique_ptr<juce::ToggleButton> phaseRButton;

    std::unique_ptr<juce::Label> widthValueLabel;
    std::unique_ptr<juce::Label> widthLabel;
    std::unique_ptr<juce::Label> lowCutLabel;
    std::unique_ptr<juce::Label> lowCutValueLabel;
    std::unique_ptr<juce::Label> phaseLLabel;
    std::unique_ptr<juce::Label> phaseRLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WidthModule)
};
