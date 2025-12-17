/*
  ==============================================================================

    VerticalLevelMeter.h
    LED-style segmented vertical level meter

    Features:
    - 24 segments (8x3px each, 2px gap)
    - Color zones: green (0-60%), yellow (60-75%), red (75-100%)
    - Smooth level decay (~50ms time constant)
    - Peak hold indicator (2 second hold)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"

/**
 * @brief LED-style segmented vertical level meter
 */
class VerticalLevelMeter : public juce::Component,
                           public juce::Timer
{
public:
    static constexpr int NUM_SEGMENTS = 24;
    static constexpr int SEGMENT_WIDTH = 8;
    static constexpr int SEGMENT_HEIGHT = 3;
    static constexpr int SEGMENT_GAP = 2;

    // Color zone boundaries (as segment indices)
    static constexpr int GREEN_END = 14;      // 0-13 (60% of 24 ≈ 14.4)
    static constexpr int YELLOW_END = 18;     // 14-17 (75% of 24 = 18)
    // Red: 18-23 (75-100%)

    static constexpr float DECAY_TIME_MS = 50.0f;
    static constexpr float PEAK_HOLD_TIME_MS = 2000.0f;

    VerticalLevelMeter()
    {
        startTimerHz(60);  // 60 FPS refresh
    }

    ~VerticalLevelMeter() override
    {
        stopTimer();
    }

    void prepare(double /*sampleRate*/)
    {
        // Could use sample rate for more precise timing if needed
    }

    /**
     * @brief Set the current input level (0.0 to 1.0)
     */
    void setLevel(float newLevel)
    {
        // Clamp input
        newLevel = juce::jlimit(0.0f, 1.0f, newLevel);

        // Update current level (instant for increases, decay handled separately)
        if (newLevel > currentLevel)
            currentLevel = newLevel;

        inputLevel = newLevel;

        // Update peak
        if (newLevel > peakLevel)
        {
            peakLevel = newLevel;
            peakHoldCounter = 0.0f;
        }
    }

    /**
     * @brief Get the current display level (after decay)
     */
    float getDisplayLevel() const
    {
        return currentLevel;
    }

    /**
     * @brief Get the current peak level
     */
    float getPeakLevel() const
    {
        return peakLevel;
    }

    /**
     * @brief Get the number of lit segments based on current level
     */
    int getLitSegmentCount() const
    {
        return static_cast<int>(currentLevel * NUM_SEGMENTS + 0.5f);
    }

    /**
     * @brief Get the total number of segments
     */
    int getSegmentCount() const
    {
        return NUM_SEGMENTS;
    }

    /**
     * @brief Get segment width in pixels
     */
    int getSegmentWidth() const
    {
        return SEGMENT_WIDTH;
    }

    /**
     * @brief Get segment height in pixels
     */
    int getSegmentHeight() const
    {
        return SEGMENT_HEIGHT;
    }

    /**
     * @brief Get gap between segments in pixels
     */
    int getSegmentGap() const
    {
        return SEGMENT_GAP;
    }

    /**
     * @brief Get the color for a specific segment index
     * @param segmentIndex 0-based index (0 = bottom, 23 = top)
     */
    juce::Colour getSegmentColor(int segmentIndex) const
    {
        if (segmentIndex < GREEN_END)
            return ReOrderColors::statusGood;      // Green: #00ff88
        else if (segmentIndex < YELLOW_END)
            return ReOrderColors::statusWarn;      // Yellow: #ffcc00
        else
            return ReOrderColors::statusError;     // Red: #ff3366
    }

    /**
     * @brief Update decay (call from timer or external source)
     * @param deltaTimeSeconds Time since last update in seconds
     */
    void updateDecay(float deltaTimeSeconds)
    {
        // Exponential decay for current level
        if (currentLevel > inputLevel)
        {
            float decayCoeff = std::exp(-deltaTimeSeconds / (DECAY_TIME_MS * 0.001f));
            currentLevel = inputLevel + (currentLevel - inputLevel) * decayCoeff;

            // Snap to zero if very small
            if (currentLevel < 0.001f)
                currentLevel = 0.0f;
        }

        // Peak hold countdown
        peakHoldCounter += deltaTimeSeconds * 1000.0f;  // Convert to ms
        if (peakHoldCounter > PEAK_HOLD_TIME_MS)
        {
            // Start decaying peak
            float peakDecayCoeff = std::exp(-deltaTimeSeconds / (DECAY_TIME_MS * 0.002f));  // Slower decay for peak
            peakLevel *= peakDecayCoeff;

            if (peakLevel < 0.001f)
                peakLevel = 0.0f;
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(2);

        int totalHeight = NUM_SEGMENTS * SEGMENT_HEIGHT + (NUM_SEGMENTS - 1) * SEGMENT_GAP;
        int startY = bounds.getBottom() - totalHeight;

        int litSegments = getLitSegmentCount();
        int peakSegment = static_cast<int>(peakLevel * NUM_SEGMENTS);

        for (int i = 0; i < NUM_SEGMENTS; ++i)
        {
            int y = startY + (NUM_SEGMENTS - 1 - i) * (SEGMENT_HEIGHT + SEGMENT_GAP);
            juce::Rectangle<int> segmentBounds(
                bounds.getX() + (bounds.getWidth() - SEGMENT_WIDTH) / 2,
                y,
                SEGMENT_WIDTH,
                SEGMENT_HEIGHT
            );

            juce::Colour segmentColor = getSegmentColor(i);

            if (i < litSegments)
            {
                // Lit segment
                g.setColour(segmentColor);
                g.fillRoundedRectangle(segmentBounds.toFloat(), 1.0f);

                // Add glow effect
                g.setColour(segmentColor.withAlpha(0.3f));
                g.drawRoundedRectangle(segmentBounds.toFloat().expanded(1), 2.0f, 1.0f);
            }
            else
            {
                // Unlit segment
                g.setColour(juce::Colour(0xff222222));
                g.fillRoundedRectangle(segmentBounds.toFloat(), 1.0f);
            }

            // Peak indicator
            if (i == peakSegment && peakLevel > 0.0f)
            {
                g.setColour(segmentColor);
                g.fillRoundedRectangle(segmentBounds.toFloat(), 1.0f);
            }
        }
    }

    void resized() override
    {
        // Component handles its own sizing
    }

private:
    void timerCallback() override
    {
        updateDecay(1.0f / 60.0f);  // Assuming 60Hz timer
        repaint();
    }

    float inputLevel = 0.0f;
    float currentLevel = 0.0f;
    float peakLevel = 0.0f;
    float peakHoldCounter = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalLevelMeter)
};
