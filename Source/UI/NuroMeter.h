#pragma once

#include <JuceHeader.h>
#include "NuroColors.h"

namespace UI
{

/**
 * @brief NuroLevelMeter - LED Ladder Style Level Meter
 *
 * Vertical or horizontal LED meter with individual glowing segments,
 * color gradient from cyan to yellow to red, and peak hold indicator.
 * Matches the Waves X-Vox Pro visual style.
 */
class NuroLevelMeter : public juce::Component
{
public:
    enum class Orientation { Horizontal, Vertical };

    NuroLevelMeter(Orientation orient = Orientation::Horizontal)
        : orientation(orient)
    {
        setOpaque(false);
    }

    void setLevel(float newLevel)
    {
        level = juce::jlimit(0.0f, 1.0f, newLevel);

        // Peak hold logic
        if (level > peakLevel)
        {
            peakLevel = level;
            peakHoldTime = juce::Time::getMillisecondCounter();
        }
        else if (juce::Time::getMillisecondCounter() - peakHoldTime > peakHoldMs)
        {
            peakLevel *= peakDecay;
        }

        repaint();
    }

    float getLevel() const { return level; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float scale = (orientation == Orientation::Horizontal)
            ? bounds.getHeight() / 12.0f
            : bounds.getWidth() / 12.0f;

        // Background
        g.setColour(NuroColors::colour(NuroColors::backgroundBlack));
        g.fillRoundedRectangle(bounds, 3.0f * scale);

        // Inner shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(1), 3.0f * scale, 1.0f);

        auto innerBounds = bounds.reduced(2 * scale);

        // Convert to dB for more musical response
        float dbLevel = level > 0.0001f ? 20.0f * std::log10(level) : -80.0f;
        float meterLevel = juce::jlimit(0.0f, 1.0f, (dbLevel + 60.0f) / 60.0f);

        float dbPeak = peakLevel > 0.0001f ? 20.0f * std::log10(peakLevel) : -80.0f;
        float peakMeterLevel = juce::jlimit(0.0f, 1.0f, (dbPeak + 60.0f) / 60.0f);

        // LED segments
        int numSegments = 32;
        float segmentGap = 1.0f * scale;

        if (orientation == Orientation::Horizontal)
        {
            float segmentWidth = (innerBounds.getWidth() - (numSegments - 1) * segmentGap) / numSegments;
            float segmentHeight = innerBounds.getHeight() - 2 * scale;
            float segY = innerBounds.getY() + scale;

            for (int i = 0; i < numSegments; ++i)
            {
                float segX = innerBounds.getX() + i * (segmentWidth + segmentGap);
                float segmentPos = static_cast<float>(i) / numSegments;
                bool isLit = segmentPos < meterLevel;
                bool isPeak = std::abs(segmentPos - peakMeterLevel) < (1.0f / numSegments) && peakMeterLevel > 0.02f;

                juce::Colour segColor = getSegmentColor(segmentPos);

                if (isLit)
                {
                    // Glowing segment
                    g.setColour(segColor.withAlpha(0.3f));
                    g.fillRect(segX - 2, segY - 1, segmentWidth + 4, segmentHeight + 2);

                    g.setColour(segColor);
                    g.fillRect(segX, segY, segmentWidth, segmentHeight);
                }
                else if (isPeak)
                {
                    // Peak indicator - bright white/cyan
                    g.setColour(NuroColors::colour(NuroColors::meterPeakCyan));
                    g.fillRect(segX, segY, segmentWidth, segmentHeight);
                }
                else
                {
                    // Unlit segment
                    g.setColour(segColor.withAlpha(0.08f));
                    g.fillRect(segX, segY, segmentWidth, segmentHeight);
                }
            }
        }
        else // Vertical
        {
            float segmentHeight = (innerBounds.getHeight() - (numSegments - 1) * segmentGap) / numSegments;
            float segmentWidth = innerBounds.getWidth() - 2 * scale;
            float segX = innerBounds.getX() + scale;

            for (int i = 0; i < numSegments; ++i)
            {
                // Bottom to top
                int segIdx = numSegments - 1 - i;
                float segY = innerBounds.getY() + i * (segmentHeight + segmentGap);
                float segmentPos = static_cast<float>(segIdx) / numSegments;
                bool isLit = segmentPos < meterLevel;
                bool isPeak = std::abs(segmentPos - peakMeterLevel) < (1.0f / numSegments) && peakMeterLevel > 0.02f;

                juce::Colour segColor = getSegmentColor(1.0f - segmentPos);

                if (isLit)
                {
                    g.setColour(segColor.withAlpha(0.3f));
                    g.fillRect(segX - 1, segY - 2, segmentWidth + 2, segmentHeight + 4);

                    g.setColour(segColor);
                    g.fillRect(segX, segY, segmentWidth, segmentHeight);
                }
                else if (isPeak)
                {
                    g.setColour(NuroColors::colour(NuroColors::meterPeakCyan));
                    g.fillRect(segX, segY, segmentWidth, segmentHeight);
                }
                else
                {
                    g.setColour(segColor.withAlpha(0.08f));
                    g.fillRect(segX, segY, segmentWidth, segmentHeight);
                }
            }
        }
    }

private:
    juce::Colour getSegmentColor(float position) const
    {
        // Cyan -> Green -> Yellow -> Orange -> Red
        if (position < 0.5f)
        {
            return NuroColors::colour(NuroColors::accentCyan).interpolatedWith(
                NuroColors::colour(NuroColors::meterGreen), position * 2.0f);
        }
        else if (position < 0.7f)
        {
            return NuroColors::colour(NuroColors::meterGreen).interpolatedWith(
                NuroColors::colour(NuroColors::meterYellow), (position - 0.5f) * 5.0f);
        }
        else if (position < 0.85f)
        {
            return NuroColors::colour(NuroColors::meterYellow).interpolatedWith(
                NuroColors::colour(NuroColors::meterOrange), (position - 0.7f) * 6.67f);
        }
        else
        {
            return NuroColors::colour(NuroColors::meterOrange).interpolatedWith(
                NuroColors::colour(NuroColors::meterRed), (position - 0.85f) * 6.67f);
        }
    }

    Orientation orientation;
    float level = 0.0f;
    float peakLevel = 0.0f;
    uint32 peakHoldTime = 0;
    static constexpr uint32 peakHoldMs = 1500;
    static constexpr float peakDecay = 0.95f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NuroLevelMeter)
};

/**
 * @brief NuroCorrelationMeter - Phase Correlation Display with LED Style
 *
 * Horizontal LED meter showing stereo correlation from -1 to +1.
 * Color-coded: Red (out of phase) -> Yellow -> Green -> Cyan (mono).
 */
class NuroCorrelationMeter : public juce::Component
{
public:
    NuroCorrelationMeter()
    {
        setOpaque(false);
    }

    void setCorrelation(float newCorrelation)
    {
        correlation = juce::jlimit(-1.0f, 1.0f, newCorrelation);
        repaint();
    }

    float getCorrelation() const { return correlation; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float scale = bounds.getHeight() / 14.0f;

        // Background
        g.setColour(NuroColors::colour(NuroColors::backgroundBlack));
        g.fillRoundedRectangle(bounds, 4.0f * scale);

        auto innerBounds = bounds.reduced(3 * scale);
        float centerX = innerBounds.getCentreX();

        // Center line
        g.setColour(NuroColors::colour(NuroColors::borderMedium));
        g.fillRect(centerX - 0.5f * scale, innerBounds.getY(), scale, innerBounds.getHeight());

        // LED segments
        int numSegments = 40;
        float segmentWidth = (innerBounds.getWidth() - (numSegments - 1) * scale) / numSegments;
        float segmentHeight = innerBounds.getHeight() - 2 * scale;
        float segY = innerBounds.getY() + scale;

        // Normalize correlation to 0-1 range for position
        float normalizedCorr = (correlation + 1.0f) / 2.0f;
        int activeSegment = static_cast<int>(normalizedCorr * numSegments);
        int centerSegment = numSegments / 2;

        for (int i = 0; i < numSegments; ++i)
        {
            float segX = innerBounds.getX() + i * (segmentWidth + scale);
            float segmentPos = static_cast<float>(i) / numSegments; // 0 = left (-1), 1 = right (+1)

            // Color based on position: red on left, green/cyan on right
            juce::Colour segColor = getCorrelationColor(segmentPos);

            // Determine if segment should be lit
            bool isLit = false;
            if (correlation >= 0)
            {
                // Positive: light from center to right
                isLit = i >= centerSegment && i <= activeSegment;
            }
            else
            {
                // Negative: light from center to left
                isLit = i <= centerSegment && i >= activeSegment;
            }

            if (isLit)
            {
                // Glow effect
                g.setColour(segColor.withAlpha(0.35f));
                g.fillRect(segX - 2, segY - 1, segmentWidth + 4, segmentHeight + 2);

                // Core segment
                g.setColour(segColor);
                g.fillRect(segX, segY, segmentWidth, segmentHeight);
            }
            else
            {
                g.setColour(segColor.withAlpha(0.06f));
                g.fillRect(segX, segY, segmentWidth, segmentHeight);
            }
        }

        // Indicator line at current position
        float indicatorX = innerBounds.getX() + innerBounds.getWidth() * normalizedCorr;
        g.setColour(juce::Colours::white);
        g.fillRect(indicatorX - scale, bounds.getY(), 2 * scale, bounds.getHeight());
    }

private:
    juce::Colour getCorrelationColor(float position) const
    {
        // position 0 = -1 (out of phase, red)
        // position 0.5 = 0 (neutral, yellow/green)
        // position 1 = +1 (in phase, cyan)
        if (position < 0.3f)
        {
            return NuroColors::colour(NuroColors::meterRed).interpolatedWith(
                NuroColors::colour(NuroColors::meterOrange), position / 0.3f);
        }
        else if (position < 0.5f)
        {
            return NuroColors::colour(NuroColors::meterOrange).interpolatedWith(
                NuroColors::colour(NuroColors::meterYellow), (position - 0.3f) / 0.2f);
        }
        else if (position < 0.7f)
        {
            return NuroColors::colour(NuroColors::meterYellow).interpolatedWith(
                NuroColors::colour(NuroColors::meterGreen), (position - 0.5f) / 0.2f);
        }
        else
        {
            return NuroColors::colour(NuroColors::meterGreen).interpolatedWith(
                NuroColors::colour(NuroColors::accentCyan), (position - 0.7f) / 0.3f);
        }
    }

    float correlation = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NuroCorrelationMeter)
};

} // namespace UI
