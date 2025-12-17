/*
  ==============================================================================

    MeterStrip.h
    Input/Output meter strip panel with L/R meters and dB readout

    Layout (matching JSX design):
    ┌─────────┐
    │ I       │
    │ N       │  ← Vertical rotated label (writingMode: vertical-rl, rotate 180°)
    │ P       │
    │ U       │
    │ T       │
    ├─────────┤
    │ ┌─┬─┐   │
    │ │ │ │   │  ← L/R meter pair (24 segments each)
    │ │ │ │   │
    │ │ │ │   │
    │ └─┴─┘   │
    │  L  R   │  ← L/R labels
    ├─────────┤
    │ 0.0dB   │  ← dB readout (green #00ff88)
    └─────────┘

    Features:
    - Background: #0d0d0d
    - Border: 1px #2a2a2a, 8px radius
    - Vertical label (rotated 180° with vertical writing)
    - L/R VerticalLevelMeter pair
    - L/R channel labels
    - dB readout at bottom in green

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

        // Create dB readout label - green color as per JSX design
        dbReadout = std::make_unique<juce::Label>("dbReadout", "0.0dB");
        dbReadout->setJustificationType(juce::Justification::centred);
        dbReadout->setColour(juce::Label::textColourId, ReOrderColors::accentOutput);  // #00ff88
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
     * @return Rotation in degrees (180 for both input and output as per JSX)
     */
    float getLabelRotationDegrees() const
    {
        return 180.0f;  // Both use rotate(180deg) in JSX
    }

    juce::String getLabelText() const
    {
        return (stripType == Type::Input) ? "INPUT" : "OUTPUT";
    }

    //==============================================================================
    // Level Methods
    //==============================================================================

    /**
     * @brief Set meter levels from LINEAR peak values (0.0 to 1.0+)
     * @param leftLinear Left channel linear peak (0.0 to 1.0+)
     * @param rightLinear Right channel linear peak (0.0 to 1.0+)
     */
    void setLevels(float leftLinear, float rightLinear)
    {
        // Convert linear to dB for display
        // Clamp to avoid log(0)
        float leftDb = (leftLinear > 0.0001f) ? 20.0f * std::log10(leftLinear) : -100.0f;
        float rightDb = (rightLinear > 0.0001f) ? 20.0f * std::log10(rightLinear) : -100.0f;

        leftLevel = leftDb;
        rightLevel = rightDb;

        // Pass linear values directly to meters (they expect 0-1 range)
        // Clamp to 0-1 for display
        leftMeter->setLevel(juce::jlimit(0.0f, 1.0f, leftLinear));
        rightMeter->setLevel(juce::jlimit(0.0f, 1.0f, rightLinear));

        // Update dB readout to show peak level
        float peakDb = std::max(leftDb, rightDb);
        dbReadout->setText(formatDbValue(peakDb), juce::dontSendNotification);
    }

    float getLeftLevel() const { return leftLevel; }
    float getRightLevel() const { return rightLevel; }

    /**
     * @brief Format dB value for display (matches JSX: "0.0dB" or "+0.0dB")
     * @param db Level in decibels
     * @return Formatted string
     */
    juce::String formatDbValue(float db) const
    {
        if (db <= INFINITY_THRESHOLD_DB)
            return "-inf";

        // Format with sign for positive values (as per JSX output meter)
        if (db >= 0.0f)
            return "+" + juce::String(db, 1) + "dB";

        return juce::String(db, 1) + "dB";
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

        // Draw horizontal label at top
        drawHorizontalLabel(g);

        // Draw L/R channel labels below meters
        drawChannelLabels(g);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        // Layout:
        // - Horizontal label at top
        // - Meters fill middle space
        // - L/R labels below meters
        // - dB readout at very bottom

        int labelHeight = 20;       // Space for INPUT/OUTPUT label at top
        int dbHeight = 16;          // dB readout height
        int channelLabelHeight = 14; // L/R labels height
        int padding = 4;

        // dB readout at very bottom
        dbReadout->setBounds(0, bounds.getBottom() - dbHeight - padding,
                            bounds.getWidth(), dbHeight);

        // Meter area: starts just below label, ends just above L/R labels
        int meterTop = labelHeight + padding;
        int meterBottom = bounds.getHeight() - dbHeight - channelLabelHeight - padding * 2;
        int meterHeight = meterBottom - meterTop;

        int meterWidth = 8;
        int meterSpacing = 4;
        int totalMeterWidth = meterWidth * 2 + meterSpacing;
        int meterX = (bounds.getWidth() - totalMeterWidth) / 2;

        leftMeter->setBounds(meterX, meterTop, meterWidth, meterHeight);
        rightMeter->setBounds(meterX + meterWidth + meterSpacing, meterTop, meterWidth, meterHeight);
    }

private:
    /**
     * @brief Draw horizontal label at the top
     */
    void drawHorizontalLabel(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        g.setColour(juce::Colour(0xff666666));  // #666 as per JSX
        g.setFont(juce::FontOptions(9.0f));

        // Draw label horizontally at top, centered
        juce::Rectangle<int> labelBounds(0, 6, bounds.getWidth(), 14);
        g.drawText(getLabelText(), labelBounds, juce::Justification::centred, false);
    }

    /**
     * @brief Draw L/R channel labels below meters
     */
    void drawChannelLabels(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        int meterWidth = 8;
        int meterSpacing = 4;
        int totalMeterWidth = meterWidth * 2 + meterSpacing;
        int meterX = (bounds.getWidth() - totalMeterWidth) / 2;

        // Position: just above the dB readout
        int dbHeight = 16;
        int padding = 4;
        int labelY = bounds.getHeight() - dbHeight - padding - 14;

        g.setColour(juce::Colour(0xff666666));  // #666 as per JSX
        g.setFont(juce::FontOptions(9.0f));

        // L label centered under left meter
        g.drawText("L", meterX - 2, labelY, meterWidth + 4, 12,
                   juce::Justification::centred, false);

        // R label centered under right meter
        g.drawText("R", meterX + meterWidth + meterSpacing - 2, labelY, meterWidth + 4, 12,
                   juce::Justification::centred, false);
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
