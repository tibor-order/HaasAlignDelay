/*
  ==============================================================================

    VerticalLevelMeter.h
    LED-style segmented vertical level meter

    Features (matching JSX LevelMeter component):
    - 24 segments
    - Each segment: 8px wide × 3px tall
    - 2px gap between segments
    - Color zones: green (0-60%), yellow (60-75%), red (75-100%)
    - Glow effect on active segments: box-shadow: 0 0 4px {color}66
    - Inactive segment color: #222222
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
    static constexpr int NUM_SEGMENTS = 48;   // More segments for higher resolution
    static constexpr int SEGMENT_WIDTH = 8;
    static constexpr int SEGMENT_HEIGHT = 3;
    static constexpr int SEGMENT_GAP = 2;

    // Color zone boundaries (as segment indices) - matching JSX percentages
    // JSX: 0-60% green, 60-75% yellow, 75-100% red
    static constexpr int GREEN_END = 29;      // segments 0-28 (60% of 48 ≈ 28.8)
    static constexpr int YELLOW_END = 36;     // segments 29-35 (75% of 48 = 36)
    // Red: segments 36-47 (75-100%)

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
     * Matches JSX colors exactly:
     *   - Green: #00ff88 (0-60%)
     *   - Yellow: #ffcc00 (60-75%)
     *   - Red: #ff3366 (75-100%)
     */
    juce::Colour getSegmentColor(int segmentIndex) const
    {
        if (segmentIndex < GREEN_END)
            return juce::Colour(0xff00ff88);      // Green: #00ff88
        else if (segmentIndex < YELLOW_END)
            return juce::Colour(0xffffcc00);      // Yellow: #ffcc00
        else
            return juce::Colour(0xffff3366);      // Red: #ff3366
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
        auto bounds = getLocalBounds();

        // Calculate segment dimensions to fill available height
        // Total height = NUM_SEGMENTS * segmentHeight + (NUM_SEGMENTS - 1) * gap
        // We want this to equal bounds.getHeight()
        float availableHeight = static_cast<float>(bounds.getHeight());
        float totalGapHeight = static_cast<float>((NUM_SEGMENTS - 1) * SEGMENT_GAP);
        float segmentHeight = (availableHeight - totalGapHeight) / static_cast<float>(NUM_SEGMENTS);

        // Ensure minimum segment height
        segmentHeight = std::max(segmentHeight, 2.0f);

        int litSegments = getLitSegmentCount();
        int peakSegment = static_cast<int>(peakLevel * NUM_SEGMENTS);

        // Center the segments horizontally
        float segmentX = static_cast<float>(bounds.getX() + (bounds.getWidth() - SEGMENT_WIDTH) / 2);

        for (int i = 0; i < NUM_SEGMENTS; ++i)
        {
            // Calculate Y position (segment 0 at bottom, segment 23 at top)
            // Draw from top to bottom, but index from bottom
            float y = static_cast<float>(bounds.getY()) +
                      static_cast<float>(NUM_SEGMENTS - 1 - i) * (segmentHeight + SEGMENT_GAP);

            juce::Rectangle<float> segmentBounds(
                segmentX,
                y,
                static_cast<float>(SEGMENT_WIDTH),
                segmentHeight
            );

            juce::Colour segmentColor = getSegmentColor(i);

            bool isLit = (i < litSegments) || (i == peakSegment && peakLevel > 0.0f);

            if (isLit)
            {
                // Lit segment - fill with color
                g.setColour(segmentColor);
                g.fillRoundedRectangle(segmentBounds, 1.0f);

                // Add glow effect (JSX: boxShadow: 0 0 4px {color}66)
                g.setColour(segmentColor.withAlpha(0.4f));
                g.drawRoundedRectangle(segmentBounds.expanded(1.0f), 2.0f, 1.0f);
            }
            else
            {
                // Unlit segment - JSX uses #222222
                g.setColour(juce::Colour(0xff222222));
                g.fillRoundedRectangle(segmentBounds, 1.0f);
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
