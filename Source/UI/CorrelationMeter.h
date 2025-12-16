#pragma once

#include <JuceHeader.h>
#include "VoxProLookAndFeel.h"

namespace UI
{

/**
 * @brief A stereo correlation meter component
 *
 * Displays correlation between left and right channels as a bar
 * extending from center. Positive correlation extends right,
 * negative extends left. Color indicates phase relationship.
 */
class CorrelationMeter : public juce::Component
{
public:
    CorrelationMeter()
    {
        setOpaque(false);
    }

    ~CorrelationMeter() override = default;

    /**
     * @brief Set the correlation value to display
     * @param newCorrelation Correlation coefficient (-1.0 to +1.0)
     */
    void setCorrelation(float newCorrelation)
    {
        correlation = juce::jlimit(-1.0f, 1.0f, newCorrelation);
        repaint();
    }

    /**
     * @brief Get the current correlation value
     * @return Current correlation coefficient
     */
    float getCorrelation() const { return correlation; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float scale = bounds.getHeight() / 10.0f;

        // Background
        g.setColour(juce::Colour(0xff0a0a0e));
        g.fillRoundedRectangle(bounds, 3.0f * scale);

        float innerPadding = 2.0f * scale;
        auto innerBounds = bounds.reduced(innerPadding);
        float centerX = innerBounds.getCentreX();

        // Center line
        g.setColour(juce::Colour(0xff2a2a35));
        g.drawLine(centerX, innerBounds.getY(), centerX, innerBounds.getBottom(), 1.0f * scale);

        // Calculate indicator position (-1 to +1 mapped to left to right)
        float normalizedCorr = (correlation + 1.0f) / 2.0f;
        float indicatorX = innerBounds.getX() + innerBounds.getWidth() * normalizedCorr;

        // Draw bar from center to indicator
        float barLeft = juce::jmin(centerX, indicatorX);
        float barRight = juce::jmax(centerX, indicatorX);
        float barWidth = barRight - barLeft;

        if (barWidth > 1.0f * scale)
        {
            auto barBounds = juce::Rectangle<float>(barLeft, innerBounds.getY() + 1 * scale,
                                                     barWidth, innerBounds.getHeight() - 2 * scale);

            // Color based on correlation value
            juce::Colour barColor;
            if (correlation > 0.5f)
                barColor = VoxProLookAndFeel::accentCyan;      // Strongly correlated (mono-ish)
            else if (correlation > 0.0f)
                barColor = VoxProLookAndFeel::meterGreen;      // Positive correlation
            else if (correlation > -0.3f)
                barColor = VoxProLookAndFeel::meterYellow;     // Slight phase issues
            else
                barColor = VoxProLookAndFeel::meterRed;        // Out of phase

            g.setColour(barColor);
            g.fillRoundedRectangle(barBounds, 1.0f * scale);
        }

        // Draw indicator line
        g.setColour(juce::Colours::white);
        g.drawLine(indicatorX, bounds.getY(), indicatorX, bounds.getBottom(), 2.0f * scale);
    }

private:
    float correlation = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CorrelationMeter)
};

} // namespace UI
