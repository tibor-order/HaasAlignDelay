/*
  ==============================================================================

    MeterStrip.h
    Input/Output meter strip panel with L/R meters and dB readout

    Layout:
    ┌─────────┐
    │   IN    │  ← Rotated label
    │ ┌─┬─┐   │
    │ │L│R│   │  ← L/R meter pair
    │ │ │ │   │
    │ │ │ │   │
    │ │ │ │   │
    │ └─┴─┘   │
    │ -12dB   │  ← dB readout
    └─────────┘

    Features:
    - Width: 50px
    - Background: #0d0d0d
    - Border: 1px #2a2a2a, 8px radius
    - Rotated label (180° for input, 0° for output)
    - L/R VerticalLevelMeter pair
    - dB readout at bottom

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "VerticalLevelMeter.h"
#include "ReOrderColors.h"

/**
 * @brief Meter strip component for input/output level display
 */
class MeterStrip : public juce::Component
{
public:
    //==============================================================================
    // Types
    //==============================================================================

    enum class Type
    {
        Input,
        Output
    };

    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float DEFAULT_WIDTH = 50.0f;
    static constexpr float BORDER_RADIUS = 8.0f;
    static constexpr float INFINITY_THRESHOLD_DB = -60.0f;

    //==============================================================================
    // Static Methods
    //==============================================================================

    static float getDefaultWidth() { return DEFAULT_WIDTH; }
    static float getBorderRadius() { return BORDER_RADIUS; }

    static juce::Colour getBackgroundColor() { return ReOrderColors::bgDarkest; }
    static juce::Colour getBorderColor() { return ReOrderColors::border; }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    explicit MeterStrip(Type type)
        : stripType(type),
          leftLevel(-100.0f),
          rightLevel(-100.0f)
    {
        // Create L/R meters
        leftMeter = std::make_unique<VerticalLevelMeter>();
        rightMeter = std::make_unique<VerticalLevelMeter>();
        addAndMakeVisible(leftMeter.get());
        addAndMakeVisible(rightMeter.get());

        // Create dB readout label
        dbReadout = std::make_unique<juce::Label>("dbReadout", "-inf dB");
        dbReadout->setJustificationType(juce::Justification::centred);
        dbReadout->setColour(juce::Label::textColourId, ReOrderColors::textMuted);
        dbReadout->setFont(juce::FontOptions(9.0f));
        addAndMakeVisible(dbReadout.get());
    }

    ~MeterStrip() override = default;

    //==============================================================================
    // Accessors
    //==============================================================================

    Type getType() const { return stripType; }

    VerticalLevelMeter* getLeftMeter() { return leftMeter.get(); }
    VerticalLevelMeter* getRightMeter() { return rightMeter.get(); }

    bool hasLabel() const { return true; }
    bool hasDbReadout() const { return true; }

    /**
     * @brief Get label rotation based on type
     * @return Rotation in degrees (180 for input, 0 for output)
     */
    float getLabelRotationDegrees() const
    {
        return (stripType == Type::Input) ? 180.0f : 0.0f;
    }

    juce::String getLabelText() const
    {
        return (stripType == Type::Input) ? "IN" : "OUT";
    }

    //==============================================================================
    // Level Methods
    //==============================================================================

    void setLevels(float leftDb, float rightDb)
    {
        leftLevel = leftDb;
        rightLevel = rightDb;

        leftMeter->setLevel(leftDb);
        rightMeter->setLevel(rightDb);

        // Update dB readout to show peak level
        float peakDb = std::max(leftDb, rightDb);
        dbReadout->setText(formatDbValue(peakDb), juce::dontSendNotification);

        repaint();
    }

    float getLeftLevel() const { return leftLevel; }
    float getRightLevel() const { return rightLevel; }

    /**
     * @brief Format dB value for display
     * @param db Level in decibels
     * @return Formatted string ("-12dB" or "-inf dB")
     */
    juce::String formatDbValue(float db) const
    {
        if (db <= INFINITY_THRESHOLD_DB)
            return "-inf dB";

        return juce::String(static_cast<int>(std::round(db))) + " dB";
    }

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Draw background
        g.setColour(getBackgroundColor());
        g.fillRoundedRectangle(bounds, BORDER_RADIUS);

        // Draw border
        g.setColour(getBorderColor());
        g.drawRoundedRectangle(bounds.reduced(0.5f), BORDER_RADIUS, 1.0f);

        // Draw rotated label
        drawLabel(g);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        int padding = 5;

        // Reserve space for label at top
        int labelHeight = 30;

        // Reserve space for dB readout at bottom
        int dbHeight = 20;
        dbReadout->setBounds(0, bounds.getBottom() - dbHeight - padding,
                            bounds.getWidth(), dbHeight);

        // Meter area
        int meterTop = labelHeight + padding;
        int meterBottom = bounds.getHeight() - dbHeight - padding * 2;
        int meterHeight = meterBottom - meterTop;

        int meterWidth = 8;
        int meterSpacing = 4;
        int totalMeterWidth = meterWidth * 2 + meterSpacing;
        int meterX = (bounds.getWidth() - totalMeterWidth) / 2;

        leftMeter->setBounds(meterX, meterTop, meterWidth, meterHeight);
        rightMeter->setBounds(meterX + meterWidth + meterSpacing, meterTop, meterWidth, meterHeight);
    }

private:
    void drawLabel(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        // Label area at top
        auto labelBounds = juce::Rectangle<float>(0.0f, 5.0f, static_cast<float>(bounds.getWidth()), 25.0f);

        g.setColour(ReOrderColors::textMuted);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));

        if (stripType == Type::Input)
        {
            // Rotate 180° for input label
            juce::AffineTransform rotation = juce::AffineTransform::rotation(
                juce::MathConstants<float>::pi,
                labelBounds.getCentreX(),
                labelBounds.getCentreY());

            g.addTransform(rotation);
            g.drawText(getLabelText(), labelBounds, juce::Justification::centred, false);
            g.addTransform(rotation.inverted());
        }
        else
        {
            // Normal orientation for output
            g.drawText(getLabelText(), labelBounds, juce::Justification::centred, false);
        }
    }

    //==============================================================================
    // Members
    //==============================================================================

    Type stripType;

    std::unique_ptr<VerticalLevelMeter> leftMeter;
    std::unique_ptr<VerticalLevelMeter> rightMeter;
    std::unique_ptr<juce::Label> dbReadout;

    float leftLevel;
    float rightLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterStrip)
};
