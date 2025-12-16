#pragma once

#include <JuceHeader.h>
#include "VoxProLookAndFeel.h"

namespace UI
{

/**
 * @brief A segmented level meter component with gradient coloring
 *
 * Displays audio levels as a horizontal bar of colored segments,
 * transitioning from green through yellow to red.
 */
class LevelMeter : public juce::Component
{
public:
    LevelMeter()
    {
        setOpaque(false);
    }

    ~LevelMeter() override = default;

    /**
     * @brief Set the current level to display
     * @param newLevel Linear level (0.0 to 1.0+)
     */
    void setLevel(float newLevel)
    {
        level = newLevel;
        repaint();
    }

    /**
     * @brief Get the current level
     * @return Current linear level
     */
    float getLevel() const { return level; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float scale = bounds.getHeight() / 10.0f; // Reference height

        // Background
        g.setColour(juce::Colour(0xff0a0a0e));
        g.fillRoundedRectangle(bounds, 3.0f * scale);

        // Convert to dB and normalize
        float clampedLevel = juce::jlimit(0.0f, 1.0f, level);
        float dbLevel = clampedLevel > 0.0001f ? 20.0f * std::log10(clampedLevel) : -80.0f;
        float meterLevel = juce::jlimit(0.0f, 1.0f, (dbLevel + 60.0f) / 60.0f);

        auto innerBounds = bounds.reduced(2 * scale);

        int numSegments = 40;
        float segmentWidth = (innerBounds.getWidth() - (numSegments - 1) * 1.0f * scale) / numSegments;
        float segmentHeight = innerBounds.getHeight() - 2 * scale;
        float segY = innerBounds.getY() + 1 * scale;

        for (int i = 0; i < numSegments; ++i)
        {
            float segX = innerBounds.getX() + i * (segmentWidth + 1.0f * scale);
            float segmentPos = static_cast<float>(i) / numSegments;
            bool isLit = segmentPos < meterLevel;

            juce::Colour segColor;
            if (segmentPos < 0.5f)
                segColor = VoxProLookAndFeel::meterGreen.interpolatedWith(VoxProLookAndFeel::accentCyan, segmentPos * 2.0f);
            else if (segmentPos < 0.7f)
                segColor = VoxProLookAndFeel::accentCyan.interpolatedWith(VoxProLookAndFeel::meterYellow, (segmentPos - 0.5f) * 5.0f);
            else if (segmentPos < 0.85f)
                segColor = VoxProLookAndFeel::meterYellow.interpolatedWith(juce::Colour(0xffff6600), (segmentPos - 0.7f) * 6.67f);
            else
                segColor = juce::Colour(0xffff6600).interpolatedWith(VoxProLookAndFeel::meterRed, (segmentPos - 0.85f) * 6.67f);

            if (isLit)
            {
                g.setColour(segColor);
                g.fillRect(segX, segY, segmentWidth, segmentHeight);
            }
            else
            {
                g.setColour(segColor.withAlpha(0.08f));
                g.fillRect(segX, segY, segmentWidth, segmentHeight);
            }
        }
    }

private:
    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

} // namespace UI
