/*
  ==============================================================================

    ModuleIcons.h
    SVG-style path icons for each module

    Features:
    - 60×60px canvas for each icon
    - Stroke-based rendering (not filled)
    - Uses module accent colors
    - Glow filter when module is active
    - Optional animation state

    Icons:
    - HaasIcon: Two offset sine waves (delay visualization)
    - WidthIcon: Expanding arrows from center point
    - PhaseIcon: Concentric circles with crossing waves
    - OutputIcon: Speaker with radiating sound waves

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"

/**
 * @brief Module icons factory class
 */
class ModuleIcons
{
public:
    static constexpr float ICON_SIZE = 60.0f;
    static constexpr float DEFAULT_STROKE_WIDTH = 2.0f;
    static constexpr float ACTIVE_GLOW_AMOUNT = 10.0f;

    enum class IconType
    {
        Haas,
        Width,
        Phase,
        Output
    };

    //==============================================================================
    // Path Generators
    //==============================================================================

    /**
     * @brief Get the Haas/Delay icon path
     * Two offset sine waves representing delay
     */
    static juce::Path getHaasIconPath()
    {
        juce::Path path;

        const float width = ICON_SIZE;
        const float height = ICON_SIZE;
        const float centerY = height / 2.0f;
        const float amplitude = height / 4.0f;
        const float offset = 8.0f;  // Horizontal offset between waves

        // First wave (original signal)
        path.startNewSubPath(5.0f, centerY);
        for (float x = 5.0f; x <= width - 5.0f; x += 1.0f)
        {
            float normalizedX = (x - 5.0f) / (width - 10.0f);
            float y = centerY - amplitude * std::sin(normalizedX * juce::MathConstants<float>::twoPi * 2.0f);
            path.lineTo(x, y);
        }

        // Second wave (delayed signal) - offset down and right
        path.startNewSubPath(5.0f + offset, centerY + 6.0f);
        for (float x = 5.0f + offset; x <= width - 5.0f; x += 1.0f)
        {
            float normalizedX = (x - 5.0f - offset) / (width - 10.0f - offset);
            float y = centerY + 6.0f - amplitude * 0.7f * std::sin(normalizedX * juce::MathConstants<float>::twoPi * 2.0f);
            path.lineTo(x, y);
        }

        return path;
    }

    /**
     * @brief Get the Width icon path
     * Expanding arrows from center point
     */
    static juce::Path getWidthIconPath()
    {
        juce::Path path;

        const float centerX = ICON_SIZE / 2.0f;
        const float centerY = ICON_SIZE / 2.0f;
        const float innerRadius = 8.0f;
        const float outerRadius = 25.0f;
        const float arrowSize = 6.0f;

        // Left arrow
        path.startNewSubPath(centerX - innerRadius, centerY);
        path.lineTo(centerX - outerRadius, centerY);
        // Arrow head
        path.lineTo(centerX - outerRadius + arrowSize, centerY - arrowSize);
        path.startNewSubPath(centerX - outerRadius, centerY);
        path.lineTo(centerX - outerRadius + arrowSize, centerY + arrowSize);

        // Right arrow
        path.startNewSubPath(centerX + innerRadius, centerY);
        path.lineTo(centerX + outerRadius, centerY);
        // Arrow head
        path.lineTo(centerX + outerRadius - arrowSize, centerY - arrowSize);
        path.startNewSubPath(centerX + outerRadius, centerY);
        path.lineTo(centerX + outerRadius - arrowSize, centerY + arrowSize);

        // Center point (small circle)
        path.addEllipse(centerX - 3.0f, centerY - 3.0f, 6.0f, 6.0f);

        // Top and bottom expansion lines
        path.startNewSubPath(centerX - innerRadius * 0.7f, centerY - innerRadius);
        path.lineTo(centerX - outerRadius * 0.6f, centerY - outerRadius * 0.6f);

        path.startNewSubPath(centerX + innerRadius * 0.7f, centerY - innerRadius);
        path.lineTo(centerX + outerRadius * 0.6f, centerY - outerRadius * 0.6f);

        path.startNewSubPath(centerX - innerRadius * 0.7f, centerY + innerRadius);
        path.lineTo(centerX - outerRadius * 0.6f, centerY + outerRadius * 0.6f);

        path.startNewSubPath(centerX + innerRadius * 0.7f, centerY + innerRadius);
        path.lineTo(centerX + outerRadius * 0.6f, centerY + outerRadius * 0.6f);

        return path;
    }

    /**
     * @brief Get the Phase icon path
     * Concentric circles with crossing waves
     */
    static juce::Path getPhaseIconPath()
    {
        juce::Path path;

        const float centerX = ICON_SIZE / 2.0f;
        const float centerY = ICON_SIZE / 2.0f;

        // Concentric circles
        path.addEllipse(centerX - 10.0f, centerY - 10.0f, 20.0f, 20.0f);
        path.addEllipse(centerX - 20.0f, centerY - 20.0f, 40.0f, 40.0f);

        // Crossing wave (horizontal)
        path.startNewSubPath(5.0f, centerY);
        for (float x = 5.0f; x <= ICON_SIZE - 5.0f; x += 1.0f)
        {
            float normalizedX = (x - 5.0f) / (ICON_SIZE - 10.0f);
            float y = centerY + 8.0f * std::sin(normalizedX * juce::MathConstants<float>::twoPi * 1.5f);
            path.lineTo(x, y);
        }

        // Phase indicator (vertical line through center)
        path.startNewSubPath(centerX, 8.0f);
        path.lineTo(centerX, ICON_SIZE - 8.0f);

        return path;
    }

    /**
     * @brief Get the Output icon path
     * Speaker with radiating sound waves
     */
    static juce::Path getOutputIconPath()
    {
        juce::Path path;

        const float centerX = ICON_SIZE / 2.0f;
        const float centerY = ICON_SIZE / 2.0f;

        // Speaker body (cone shape)
        path.startNewSubPath(12.0f, centerY - 8.0f);
        path.lineTo(22.0f, centerY - 14.0f);
        path.lineTo(22.0f, centerY + 14.0f);
        path.lineTo(12.0f, centerY + 8.0f);
        path.closeSubPath();

        // Speaker base (rectangle)
        path.addRectangle(8.0f, centerY - 6.0f, 6.0f, 12.0f);

        // Sound waves (arcs)
        float waveStartX = 28.0f;

        // First wave (small)
        path.addArc(waveStartX, centerY - 8.0f, 8.0f, 16.0f,
                    -juce::MathConstants<float>::pi * 0.4f,
                    juce::MathConstants<float>::pi * 0.4f,
                    true);

        // Second wave (medium)
        path.addArc(waveStartX + 6.0f, centerY - 12.0f, 12.0f, 24.0f,
                    -juce::MathConstants<float>::pi * 0.4f,
                    juce::MathConstants<float>::pi * 0.4f,
                    true);

        // Third wave (large)
        path.addArc(waveStartX + 12.0f, centerY - 16.0f, 16.0f, 32.0f,
                    -juce::MathConstants<float>::pi * 0.4f,
                    juce::MathConstants<float>::pi * 0.4f,
                    true);

        return path;
    }

    //==============================================================================
    // Styling
    //==============================================================================

    /**
     * @brief Get the default stroke width for icons
     */
    static float getDefaultStrokeWidth()
    {
        return DEFAULT_STROKE_WIDTH;
    }

    /**
     * @brief Get the icon color for a given icon type
     */
    static juce::Colour getIconColor(IconType type)
    {
        switch (type)
        {
            case IconType::Haas:   return ReOrderColors::accentDelay;   // Pink
            case IconType::Width:  return ReOrderColors::accentWidth;   // Cyan
            case IconType::Phase:  return ReOrderColors::accentPhase;   // Orange
            case IconType::Output: return ReOrderColors::accentOutput;  // Green
            default:               return ReOrderColors::textPrimary;
        }
    }

    /**
     * @brief Get the glow amount based on active state
     * @param isActive Whether the module is active
     * @return Glow amount in pixels (0 = no glow)
     */
    static float getGlowAmount(bool isActive)
    {
        return isActive ? ACTIVE_GLOW_AMOUNT : 0.0f;
    }

    //==============================================================================
    // Drawing Helper
    //==============================================================================

    /**
     * @brief Draw an icon with optional glow effect
     * @param g Graphics context
     * @param type Icon type
     * @param bounds Bounds to draw within
     * @param isActive Whether to show glow effect
     */
    static void drawIcon(juce::Graphics& g, IconType type, juce::Rectangle<float> bounds, bool isActive = false)
    {
        juce::Path path;

        switch (type)
        {
            case IconType::Haas:   path = getHaasIconPath(); break;
            case IconType::Width:  path = getWidthIconPath(); break;
            case IconType::Phase:  path = getPhaseIconPath(); break;
            case IconType::Output: path = getOutputIconPath(); break;
        }

        juce::Colour color = getIconColor(type);

        // Scale path to fit bounds
        auto pathBounds = path.getBounds();
        float scale = std::min(bounds.getWidth() / pathBounds.getWidth(),
                               bounds.getHeight() / pathBounds.getHeight()) * 0.9f;

        juce::AffineTransform transform = juce::AffineTransform::scale(scale)
            .translated(bounds.getCentreX() - pathBounds.getCentreX() * scale,
                        bounds.getCentreY() - pathBounds.getCentreY() * scale);

        path.applyTransform(transform);

        // Draw glow if active
        if (isActive)
        {
            g.setColour(color.withAlpha(0.3f));
            g.strokePath(path, juce::PathStrokeType(DEFAULT_STROKE_WIDTH + 4.0f));

            g.setColour(color.withAlpha(0.5f));
            g.strokePath(path, juce::PathStrokeType(DEFAULT_STROKE_WIDTH + 2.0f));
        }

        // Draw main path
        g.setColour(color);
        g.strokePath(path, juce::PathStrokeType(DEFAULT_STROKE_WIDTH,
                                                 juce::PathStrokeType::mitered,
                                                 juce::PathStrokeType::rounded));
    }

private:
    ModuleIcons() = delete;  // Static class only
};
