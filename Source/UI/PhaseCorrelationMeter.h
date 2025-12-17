/*
  ==============================================================================

    PhaseCorrelationMeter.h
    Enhanced phase correlation meter with threshold marker and correction indicator

    Features:
    - 200px × 24px dimensions
    - Scale: -1 (left) to +1 (right)
    - Gradient overlay: red → orange → yellow → green
    - Draggable threshold marker (vertical line)
    - Position indicator: 4×16px bar
    - "CORRECTING" badge when active (pulsing)
    - Value readout formatting

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"

/**
 * @brief Enhanced phase correlation meter component
 */
class PhaseCorrelationMeter : public juce::Component,
                               public juce::Timer
{
public:
    static constexpr int DEFAULT_WIDTH = 200;
    static constexpr int DEFAULT_HEIGHT = 24;
    static constexpr float SCALE_MIN = -1.0f;
    static constexpr float SCALE_MAX = 1.0f;
    static constexpr float DEFAULT_THRESHOLD = 0.3f;
    static constexpr float SMOOTHING_TIME_MS = 100.0f;
    static constexpr float PULSE_PERIOD_SECONDS = 1.0f;

    // Indicator dimensions
    static constexpr int INDICATOR_WIDTH = 4;
    static constexpr int INDICATOR_HEIGHT = 16;

    PhaseCorrelationMeter()
        : threshold(DEFAULT_THRESHOLD),
          correlation(0.0f),
          displayCorrelation(0.0f),
          correctionActive(false),
          pulsePhase(0.0f)
    {
        setSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        startTimerHz(60);  // 60 FPS for smooth animation
    }

    ~PhaseCorrelationMeter() override
    {
        stopTimer();
    }

    //==============================================================================
    // Dimensions
    //==============================================================================

    int getDefaultWidth() const { return DEFAULT_WIDTH; }
    int getDefaultHeight() const { return DEFAULT_HEIGHT; }

    //==============================================================================
    // Scale
    //==============================================================================

    float getScaleMin() const { return SCALE_MIN; }
    float getScaleMax() const { return SCALE_MAX; }

    /**
     * @brief Convert correlation value (-1 to +1) to X position
     */
    float correlationToX(float corr) const
    {
        // Map -1..+1 to 0..width
        float normalized = (corr - SCALE_MIN) / (SCALE_MAX - SCALE_MIN);
        return normalized * static_cast<float>(getWidth());
    }

    /**
     * @brief Convert X position to correlation value
     */
    float xToCorrelation(float x) const
    {
        float normalized = x / static_cast<float>(getWidth());
        return SCALE_MIN + normalized * (SCALE_MAX - SCALE_MIN);
    }

    //==============================================================================
    // Threshold
    //==============================================================================

    float getThreshold() const { return threshold; }

    void setThreshold(float newThreshold)
    {
        threshold = juce::jlimit(0.0f, 1.0f, newThreshold);
    }

    //==============================================================================
    // Correlation
    //==============================================================================

    void setCorrelation(float newCorrelation)
    {
        correlation = juce::jlimit(SCALE_MIN, SCALE_MAX, newCorrelation);
    }

    /**
     * @brief Set correlation and immediately update display (for testing)
     */
    void setCorrelationImmediate(float newCorrelation)
    {
        correlation = juce::jlimit(SCALE_MIN, SCALE_MAX, newCorrelation);
        displayCorrelation = correlation;
    }

    float getCorrelation() const { return correlation; }
    float getDisplayCorrelation() const { return displayCorrelation; }

    /**
     * @brief Get the indicator color based on correlation vs threshold
     */
    juce::Colour getIndicatorColor() const
    {
        if (displayCorrelation < threshold)
            return ReOrderColors::statusError;   // Red when below threshold
        else
            return ReOrderColors::statusGood;    // Green when above threshold
    }

    //==============================================================================
    // Correction State
    //==============================================================================

    void setCorrectionActive(bool active)
    {
        correctionActive = active;
        if (!active)
            pulsePhase = 0.0f;
    }

    bool isCorrectionActive() const { return correctionActive; }
    bool isCorrectingBadgeVisible() const { return correctionActive; }

    /**
     * @brief Get the correcting badge opacity (oscillates 0.5 to 1.0)
     */
    float getCorrectingBadgeOpacity() const
    {
        if (!correctionActive)
            return 0.0f;

        // Oscillate between 0.5 and 1.0 using sine wave
        float sine = std::sin(pulsePhase * juce::MathConstants<float>::twoPi);
        return 0.75f + 0.25f * sine;  // 0.5 to 1.0
    }

    /**
     * @brief Update animation state
     * @param deltaTimeSeconds Time since last update
     */
    void updateAnimation(float deltaTimeSeconds)
    {
        pulsePhase += deltaTimeSeconds / PULSE_PERIOD_SECONDS;
        if (pulsePhase > 1.0f)
            pulsePhase -= 1.0f;
    }

    //==============================================================================
    // Smoothing
    //==============================================================================

    /**
     * @brief Update display smoothing
     * @param deltaTimeSeconds Time since last update
     */
    void updateSmoothing(float deltaTimeSeconds)
    {
        float smoothingCoeff = std::exp(-deltaTimeSeconds / (SMOOTHING_TIME_MS * 0.001f));
        displayCorrelation = correlation + (displayCorrelation - correlation) * smoothingCoeff;
    }

    //==============================================================================
    // Value Readout
    //==============================================================================

    /**
     * @brief Get formatted value readout string
     * @return String in format "+0.65" or "-0.32"
     */
    juce::String getValueReadout() const
    {
        char buffer[16];
        if (displayCorrelation >= 0.0f)
            snprintf(buffer, sizeof(buffer), "+%.2f", displayCorrelation);
        else
            snprintf(buffer, sizeof(buffer), "%.2f", displayCorrelation);
        return juce::String(buffer);
    }

    //==============================================================================
    // Paint
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colour(0xff0a0a0a));
        g.fillRoundedRectangle(bounds, 4.0f);

        // Border
        g.setColour(ReOrderColors::border);
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

        // Gradient overlay (red → orange → yellow → green at 30% opacity)
        {
            juce::ColourGradient gradient(
                ReOrderColors::statusError.withAlpha(0.3f), bounds.getX(), 0.0f,
                ReOrderColors::statusGood.withAlpha(0.3f), bounds.getRight(), 0.0f,
                false);
            gradient.addColour(0.33, ReOrderColors::accentPhase.withAlpha(0.3f));  // Orange
            gradient.addColour(0.66, ReOrderColors::statusWarn.withAlpha(0.3f));   // Yellow

            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds.reduced(1.0f), 3.0f);
        }

        // Threshold marker (vertical line)
        float thresholdX = correlationToX(threshold);
        g.setColour(ReOrderColors::textMuted);
        g.drawVerticalLine(static_cast<int>(thresholdX), bounds.getY() + 2, bounds.getBottom() - 2);

        // Center marker (0.0)
        float centerX = correlationToX(0.0f);
        g.setColour(ReOrderColors::textDim.withAlpha(0.5f));
        g.drawVerticalLine(static_cast<int>(centerX), bounds.getY() + 2, bounds.getBottom() - 2);

        // Position indicator (4×16px bar)
        float indicatorX = correlationToX(displayCorrelation);
        float indicatorY = (bounds.getHeight() - INDICATOR_HEIGHT) / 2.0f;

        juce::Rectangle<float> indicatorBounds(
            indicatorX - INDICATOR_WIDTH / 2.0f,
            indicatorY,
            static_cast<float>(INDICATOR_WIDTH),
            static_cast<float>(INDICATOR_HEIGHT));

        juce::Colour indicatorColor = getIndicatorColor();
        g.setColour(indicatorColor);
        g.fillRoundedRectangle(indicatorBounds, 2.0f);

        // Glow effect on indicator
        g.setColour(indicatorColor.withAlpha(0.4f));
        g.drawRoundedRectangle(indicatorBounds.expanded(2.0f), 3.0f, 2.0f);

        // Scale markers
        g.setColour(ReOrderColors::textDim);
        g.setFont(9.0f);

        // -1 label
        g.drawText("-1", juce::Rectangle<float>(2, bounds.getBottom() - 10, 15, 10),
                   juce::Justification::centredLeft, false);

        // 0 label
        g.drawText("0", juce::Rectangle<float>(centerX - 5, bounds.getBottom() - 10, 10, 10),
                   juce::Justification::centred, false);

        // +1 label
        g.drawText("+1", juce::Rectangle<float>(bounds.getRight() - 17, bounds.getBottom() - 10, 15, 10),
                   juce::Justification::centredRight, false);
    }

    void resized() override
    {
        // Component handles its own sizing
    }

    //==============================================================================
    // Mouse Interaction (for threshold dragging)
    //==============================================================================

    void mouseDown(const juce::MouseEvent& event) override
    {
        // Check if click is near threshold marker
        float thresholdX = correlationToX(threshold);
        if (std::abs(event.x - thresholdX) < 10.0f)
        {
            isDraggingThreshold = true;
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (isDraggingThreshold)
        {
            float newCorrelation = xToCorrelation(static_cast<float>(event.x));
            // Threshold is only positive (0 to 1) but marker shows on the positive side
            setThreshold(std::abs(newCorrelation));
            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        isDraggingThreshold = false;
    }

private:
    void timerCallback() override
    {
        float deltaTime = 1.0f / 60.0f;
        updateSmoothing(deltaTime);
        updateAnimation(deltaTime);
        repaint();
    }

    float threshold;
    float correlation;
    float displayCorrelation;
    bool correctionActive;
    float pulsePhase;
    bool isDraggingThreshold = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseCorrelationMeter)
};
