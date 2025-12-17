/*
  ==============================================================================

    DelayModule.h
    Delay module panel with linked sliders

    Layout:
    [●] ──── DELAY ────
          [Haas Icon]
       [L Slider] [R Slider]
         0.0ms      15.0ms
         LEFT       RIGHT
           [LINK]

    Features:
    - Two vertical sliders (0-50ms each)
    - LINK toggle button
    - Link logic: dragging one slider adjusts both, maintaining offset
    - Pink accent color (#ff66aa)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModuleComponent.h"
#include "ModuleIcons.h"
#include "ReOrderColors.h"

/**
 * @brief Delay module component with linked L/R delay sliders
 */
class DelayModule : public ModuleComponent,
                    private juce::Slider::Listener
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float DELAY_MIN_MS = 0.0f;
    static constexpr float DELAY_MAX_MS = 50.0f;
    static constexpr float DELAY_DEFAULT_MS = 0.0f;

    //==============================================================================
    // Parameter IDs
    //==============================================================================

    static juce::String getLeftDelayParamID() { return "delayLeft"; }
    static juce::String getRightDelayParamID() { return "delayRight"; }
    static juce::String getLinkParamID() { return "delayLink"; }
    static juce::String getBypassParamID() { return "delayBypass"; }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    DelayModule()
        : ModuleComponent("DELAY", ReOrderColors::accentDelay),
          linkEnabled(false),
          linkOffset(0.0)
    {
        // Create left delay slider
        leftDelaySlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical,
                                                          juce::Slider::NoTextBox);
        leftDelaySlider->setRange(DELAY_MIN_MS, DELAY_MAX_MS, 0.1);
        leftDelaySlider->setValue(DELAY_DEFAULT_MS);
        leftDelaySlider->addListener(this);
        addAndMakeVisible(leftDelaySlider.get());

        // Create right delay slider
        rightDelaySlider = std::make_unique<juce::Slider>(juce::Slider::LinearVertical,
                                                           juce::Slider::NoTextBox);
        rightDelaySlider->setRange(DELAY_MIN_MS, DELAY_MAX_MS, 0.1);
        rightDelaySlider->setValue(DELAY_DEFAULT_MS);
        rightDelaySlider->addListener(this);
        addAndMakeVisible(rightDelaySlider.get());

        // Create link button
        linkButton = std::make_unique<juce::TextButton>("LINK");
        linkButton->setClickingTogglesState(true);
        linkButton->onClick = [this]()
        {
            setLinkEnabled(linkButton->getToggleState());
        };
        addAndMakeVisible(linkButton.get());

        // Create value labels
        leftValueLabel = std::make_unique<juce::Label>("leftValue", "0.0ms");
        leftValueLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(leftValueLabel.get());

        rightValueLabel = std::make_unique<juce::Label>("rightValue", "0.0ms");
        rightValueLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(rightValueLabel.get());

        // Create channel labels
        leftChannelLabel = std::make_unique<juce::Label>("leftChannel", "LEFT");
        leftChannelLabel->setJustificationType(juce::Justification::centred);
        leftChannelLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(leftChannelLabel.get());

        rightChannelLabel = std::make_unique<juce::Label>("rightChannel", "RIGHT");
        rightChannelLabel->setJustificationType(juce::Justification::centred);
        rightChannelLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(rightChannelLabel.get());

        updateValueLabels();
    }

    ~DelayModule() override = default;

    //==============================================================================
    // Slider Accessors
    //==============================================================================

    juce::Slider* getLeftDelaySlider() { return leftDelaySlider.get(); }
    juce::Slider* getRightDelaySlider() { return rightDelaySlider.get(); }
    juce::Button* getLinkButton() { return linkButton.get(); }

    //==============================================================================
    // Link Control
    //==============================================================================

    void setLinkEnabled(bool enabled)
    {
        if (enabled && !linkEnabled)
        {
            // Capture current offset when enabling link
            linkOffset = rightDelaySlider->getValue() - leftDelaySlider->getValue();
        }
        linkEnabled = enabled;
        linkButton->setToggleState(enabled, juce::dontSendNotification);
    }

    bool isLinkEnabled() const { return linkEnabled; }

    //==============================================================================
    // Icon
    //==============================================================================

    bool hasIcon() const { return true; }

    //==============================================================================
    // Value Text
    //==============================================================================

    juce::String getLeftValueText() const
    {
        return juce::String(leftDelaySlider->getValue(), 1) + "ms";
    }

    juce::String getRightValueText() const
    {
        return juce::String(rightDelaySlider->getValue(), 1) + "ms";
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
        ModuleIcons::drawIcon(g, ModuleIcons::IconType::Haas, iconBounds, getActive());
    }

    void resized() override
    {
        ModuleComponent::resized();

        auto controlsBounds = getControlsBounds();
        int padding = 10;

        // Calculate slider area
        int sliderAreaTop = controlsBounds.getY() + padding;
        int sliderHeight = controlsBounds.getHeight() - 100;  // Leave room for labels and button
        int sliderWidth = 30;
        int centerX = controlsBounds.getCentreX();
        int spacing = 40;

        // Position sliders
        leftDelaySlider->setBounds(centerX - spacing - sliderWidth / 2,
                                   sliderAreaTop,
                                   sliderWidth,
                                   sliderHeight);

        rightDelaySlider->setBounds(centerX + spacing - sliderWidth / 2,
                                    sliderAreaTop,
                                    sliderWidth,
                                    sliderHeight);

        // Position value labels below sliders
        int labelY = sliderAreaTop + sliderHeight + 5;
        int labelWidth = 60;
        int labelHeight = 20;

        leftValueLabel->setBounds(centerX - spacing - labelWidth / 2, labelY, labelWidth, labelHeight);
        rightValueLabel->setBounds(centerX + spacing - labelWidth / 2, labelY, labelWidth, labelHeight);

        // Position channel labels
        int channelLabelY = labelY + labelHeight;
        leftChannelLabel->setBounds(centerX - spacing - labelWidth / 2, channelLabelY, labelWidth, labelHeight);
        rightChannelLabel->setBounds(centerX + spacing - labelWidth / 2, channelLabelY, labelWidth, labelHeight);

        // Position link button
        int buttonWidth = 60;
        int buttonHeight = 24;
        int buttonY = channelLabelY + labelHeight + 5;
        linkButton->setBounds(centerX - buttonWidth / 2, buttonY, buttonWidth, buttonHeight);
    }

private:
    //==============================================================================
    // Slider::Listener
    //==============================================================================

    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == leftDelaySlider.get())
            onLeftSliderChanged();
        else if (slider == rightDelaySlider.get())
            onRightSliderChanged();
    }

    //==============================================================================
    // Slider Callbacks
    //==============================================================================

    void onLeftSliderChanged()
    {
        if (linkEnabled && !isUpdatingFromLink)
        {
            isUpdatingFromLink = true;
            double newRightValue = leftDelaySlider->getValue() + linkOffset;
            newRightValue = juce::jlimit(0.0, 50.0, newRightValue);
            rightDelaySlider->setValue(newRightValue, juce::dontSendNotification);
            isUpdatingFromLink = false;
        }
        updateValueLabels();
    }

    void onRightSliderChanged()
    {
        if (linkEnabled && !isUpdatingFromLink)
        {
            isUpdatingFromLink = true;
            double newLeftValue = rightDelaySlider->getValue() - linkOffset;
            newLeftValue = juce::jlimit(0.0, 50.0, newLeftValue);
            leftDelaySlider->setValue(newLeftValue, juce::dontSendNotification);
            isUpdatingFromLink = false;
        }
        updateValueLabels();
    }

    void updateValueLabels()
    {
        leftValueLabel->setText(getLeftValueText(), juce::dontSendNotification);
        rightValueLabel->setText(getRightValueText(), juce::dontSendNotification);

        // Set color based on active state
        juce::Colour textColor = getActive() ? getAccentColor() : ReOrderColors::textDim;
        leftValueLabel->setColour(juce::Label::textColourId, textColor);
        rightValueLabel->setColour(juce::Label::textColourId, textColor);
    }

    //==============================================================================
    // Members
    //==============================================================================

    std::unique_ptr<juce::Slider> leftDelaySlider;
    std::unique_ptr<juce::Slider> rightDelaySlider;
    std::unique_ptr<juce::TextButton> linkButton;
    std::unique_ptr<juce::Label> leftValueLabel;
    std::unique_ptr<juce::Label> rightValueLabel;
    std::unique_ptr<juce::Label> leftChannelLabel;
    std::unique_ptr<juce::Label> rightChannelLabel;

    bool linkEnabled;
    double linkOffset;
    bool isUpdatingFromLink = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayModule)
};
