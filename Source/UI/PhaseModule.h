/*
  ==============================================================================

    PhaseModule.h
    Auto Phase module panel with correlation meter and correction indicator

    Layout:
    [●] ── AUTO PHASE ── [EXCLUSIVE]
            [Phase Icon]

      -1        0        +1
      [━━━━━━━|━━━━●━━━━━━]
             0.65

          [AUTO FIX ON]

      [Threshold]  [Speed]
         0.30        50%
       THRESHOLD    SPEED

    Features:
    - Correlation meter with threshold
    - AUTO FIX toggle
    - Threshold knob (0-1.0)
    - Speed knob (0-100%)
    - "CORRECTING" badge when active (pulse animation)
    - Orange accent color (#ff9933)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModuleComponent.h"
#include "ModuleIcons.h"
#include "PhaseCorrelationMeter.h"
#include "ReOrderColors.h"

/**
 * @brief Phase module component with auto phase correction controls
 */
class PhaseModule : public ModuleComponent
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float THRESHOLD_MIN = 0.0f;
    static constexpr float THRESHOLD_MAX = 1.0f;
    static constexpr float THRESHOLD_DEFAULT = 0.3f;

    static constexpr float SPEED_MIN = 0.0f;
    static constexpr float SPEED_MAX = 100.0f;
    static constexpr float SPEED_DEFAULT = 50.0f;

    //==============================================================================
    // Parameter IDs
    //==============================================================================

    static juce::String getThresholdParamID() { return "phaseThreshold"; }
    static juce::String getSpeedParamID() { return "correctionSpeed"; }
    static juce::String getAutoFixParamID() { return "autoPhaseCorrection"; }
    static juce::String getBypassParamID() { return "phaseBypass"; }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    PhaseModule()
        : ModuleComponent("AUTO PHASE", ReOrderColors::accentPhase),
          isCorrecting(false),
          pulsePhase(0.0f)
    {
        // Create correlation meter
        correlationMeter = std::make_unique<PhaseCorrelationMeter>();
        addAndMakeVisible(correlationMeter.get());

        // Create AUTO FIX button
        autoFixButton = std::make_unique<juce::TextButton>("AUTO FIX ON");
        autoFixButton->setClickingTogglesState(true);
        autoFixButton->setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(autoFixButton.get());

        // Create threshold knob
        thresholdKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                        juce::Slider::NoTextBox);
        thresholdKnob->setRange(THRESHOLD_MIN, THRESHOLD_MAX, 0.01);
        thresholdKnob->setValue(THRESHOLD_DEFAULT);
        thresholdKnob->setColour(juce::Slider::rotarySliderFillColourId, getAccentColor());
        thresholdKnob->onValueChange = [this]()
        {
            correlationMeter->setThreshold(static_cast<float>(thresholdKnob->getValue()));
            updateThresholdLabel();
        };
        addAndMakeVisible(thresholdKnob.get());

        // Create speed knob
        speedKnob = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                    juce::Slider::NoTextBox);
        speedKnob->setRange(SPEED_MIN, SPEED_MAX, 1.0);
        speedKnob->setValue(SPEED_DEFAULT);
        speedKnob->setColour(juce::Slider::rotarySliderFillColourId, getAccentColor());
        speedKnob->onValueChange = [this]() { updateSpeedLabel(); };
        addAndMakeVisible(speedKnob.get());

        // Create labels
        thresholdValueLabel = std::make_unique<juce::Label>("thresholdValue", "0.30");
        thresholdValueLabel->setJustificationType(juce::Justification::centred);
        thresholdValueLabel->setColour(juce::Label::textColourId, getAccentColor());
        addAndMakeVisible(thresholdValueLabel.get());

        thresholdLabel = std::make_unique<juce::Label>("thresholdLabel", "THRESHOLD");
        thresholdLabel->setJustificationType(juce::Justification::centred);
        thresholdLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(thresholdLabel.get());

        speedValueLabel = std::make_unique<juce::Label>("speedValue", "50%");
        speedValueLabel->setJustificationType(juce::Justification::centred);
        speedValueLabel->setColour(juce::Label::textColourId, getAccentColor());
        addAndMakeVisible(speedValueLabel.get());

        speedLabel = std::make_unique<juce::Label>("speedLabel", "SPEED");
        speedLabel->setJustificationType(juce::Justification::centred);
        speedLabel->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        addAndMakeVisible(speedLabel.get());
    }

    ~PhaseModule() override = default;

    //==============================================================================
    // Control Accessors
    //==============================================================================

    PhaseCorrelationMeter* getCorrelationMeter() { return correlationMeter.get(); }
    juce::Button* getAutoFixButton() { return autoFixButton.get(); }
    juce::Slider* getThresholdKnob() { return thresholdKnob.get(); }
    juce::Slider* getSpeedKnob() { return speedKnob.get(); }

    //==============================================================================
    // Icon & Badge
    //==============================================================================

    bool hasIcon() const { return true; }
    bool hasExclusiveBadge() const { return true; }

    //==============================================================================
    // Correction State
    //==============================================================================

    void setCorrecting(bool shouldCorrect)
    {
        isCorrecting = shouldCorrect;
        setActive(shouldCorrect || getActive());

        // Update module glow state based on correction
        if (shouldCorrect)
        {
            // Override the base active glow
        }
        repaint();
    }

    bool getCorrecting() const { return isCorrecting; }

    bool isCorrectingBadgeVisible() const { return isCorrecting; }

    /**
     * @brief Get correcting badge opacity at a given time
     * @param timeSeconds Current animation time
     * @return Opacity (0.5 to 1.0)
     */
    float getCorrectingBadgeOpacity(float timeSeconds) const
    {
        // Pulse between 0.5 and 1.0 with 1 second period
        float phase = std::fmod(timeSeconds, 1.0f);
        return 0.5f + 0.5f * std::sin(phase * juce::MathConstants<float>::twoPi);
    }

    //==============================================================================
    // Override hasActiveGlow
    //==============================================================================

    bool hasActiveGlow() const
    {
        return isCorrecting;
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
        float iconSize = juce::jmin(iconBounds.getWidth(), iconBounds.getHeight()) * 0.6f;
        auto iconRect = iconBounds.withSizeKeepingCentre(iconSize, iconSize);
        iconRect.setY(iconBounds.getY() + 5);
        ModuleIcons::drawIcon(g, ModuleIcons::IconType::Phase, iconRect, isCorrecting);

        // Draw EXCLUSIVE badge in header
        drawExclusiveBadge(g);

        // Draw CORRECTING badge if active
        if (isCorrecting)
        {
            drawCorrectingBadge(g);
        }
    }

    void resized() override
    {
        ModuleComponent::resized();

        auto controlsBounds = getControlsBounds();
        int padding = 10;
        int centerX = controlsBounds.getCentreX();

        // Correlation meter
        int meterWidth = 180;
        int meterHeight = 24;
        int meterY = controlsBounds.getY() + padding;
        correlationMeter->setBounds(centerX - meterWidth / 2, meterY, meterWidth, meterHeight);

        // AUTO FIX button
        int buttonWidth = 100;
        int buttonHeight = 28;
        int buttonY = meterY + meterHeight + 20;
        autoFixButton->setBounds(centerX - buttonWidth / 2, buttonY, buttonWidth, buttonHeight);

        // Knobs section
        int knobSize = 55;
        int knobY = buttonY + buttonHeight + 20;
        int knobSpacing = 50;

        // Threshold knob
        thresholdKnob->setBounds(centerX - knobSpacing - knobSize / 2, knobY, knobSize, knobSize);

        // Speed knob
        speedKnob->setBounds(centerX + knobSpacing - knobSize / 2, knobY, knobSize, knobSize);

        // Value labels
        int labelHeight = 18;
        int valueLabelY = knobY + knobSize + 2;
        thresholdValueLabel->setBounds(centerX - knobSpacing - 30, valueLabelY, 60, labelHeight);
        speedValueLabel->setBounds(centerX + knobSpacing - 30, valueLabelY, 60, labelHeight);

        // Param labels
        int paramLabelY = valueLabelY + labelHeight;
        thresholdLabel->setBounds(centerX - knobSpacing - 45, paramLabelY, 90, labelHeight);
        speedLabel->setBounds(centerX + knobSpacing - 30, paramLabelY, 60, labelHeight);
    }

private:
    void drawExclusiveBadge(juce::Graphics& g)
    {
        auto headerBounds = getHeaderBounds().toFloat();

        // Badge in top right of header
        float badgeWidth = 65.0f;
        float badgeHeight = 16.0f;
        float badgeX = headerBounds.getRight() - badgeWidth - 10.0f;
        float badgeY = headerBounds.getCentreY() - badgeHeight / 2.0f;

        auto badgeBounds = juce::Rectangle<float>(badgeX, badgeY, badgeWidth, badgeHeight);

        g.setColour(getAccentColor().withAlpha(0.2f));
        g.fillRoundedRectangle(badgeBounds, 3.0f);

        g.setColour(getAccentColor());
        g.drawRoundedRectangle(badgeBounds, 3.0f, 1.0f);

        g.setFont(juce::FontOptions(9.0f));
        g.drawText("EXCLUSIVE", badgeBounds, juce::Justification::centred, false);
    }

    void drawCorrectingBadge(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        // Badge at bottom center
        float badgeWidth = 90.0f;
        float badgeHeight = 20.0f;
        float badgeX = bounds.getCentreX() - badgeWidth / 2.0f;
        float badgeY = bounds.getBottom() - badgeHeight - 10.0f;

        auto badgeBounds = juce::Rectangle<float>(badgeX, badgeY, badgeWidth, badgeHeight);

        float opacity = getCorrectingBadgeOpacity(pulsePhase);

        g.setColour(getAccentColor().withAlpha(opacity * 0.3f));
        g.fillRoundedRectangle(badgeBounds, 4.0f);

        g.setColour(getAccentColor().withAlpha(opacity));
        g.drawRoundedRectangle(badgeBounds, 4.0f, 1.0f);

        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("CORRECTING", badgeBounds, juce::Justification::centred, false);
    }

    void updateThresholdLabel()
    {
        thresholdValueLabel->setText(juce::String(thresholdKnob->getValue(), 2),
                                     juce::dontSendNotification);
    }

    void updateSpeedLabel()
    {
        speedValueLabel->setText(juce::String(static_cast<int>(speedKnob->getValue())) + "%",
                                 juce::dontSendNotification);
    }

    //==============================================================================
    // Members
    //==============================================================================

    std::unique_ptr<PhaseCorrelationMeter> correlationMeter;
    std::unique_ptr<juce::TextButton> autoFixButton;
    std::unique_ptr<juce::Slider> thresholdKnob;
    std::unique_ptr<juce::Slider> speedKnob;

    std::unique_ptr<juce::Label> thresholdValueLabel;
    std::unique_ptr<juce::Label> thresholdLabel;
    std::unique_ptr<juce::Label> speedValueLabel;
    std::unique_ptr<juce::Label> speedLabel;

    bool isCorrecting;
    float pulsePhase;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseModule)
};
