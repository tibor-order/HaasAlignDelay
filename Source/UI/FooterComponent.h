/*
  ==============================================================================

    FooterComponent.h
    Plugin footer with centered branding

    Layout:
    ┌─────────────────────────────────────────────────────────────┐
    │                    REORDER AUDIO                            │
    └─────────────────────────────────────────────────────────────┘

    Features:
    - Height: 40px
    - Background: #0d0d0d (bgDarkest)
    - "REORDER" in #444444
    - "AUDIO" in cyan (#00d4ff)
    - Text centered

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"

/**
 * @brief Footer component with centered branding
 */
class FooterComponent : public juce::Component
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float DEFAULT_HEIGHT = 40.0f;

    //==============================================================================
    // Static Methods
    //==============================================================================

    static float getDefaultHeight() { return DEFAULT_HEIGHT; }

    static juce::Colour getBackgroundColor()
    {
        return ReOrderColors::bgDarkest;
    }

    static juce::Colour getReorderTextColor()
    {
        return juce::Colour(0xff444444);
    }

    static juce::Colour getAudioTextColor()
    {
        return ReOrderColors::accentWidth;  // Cyan #00d4ff
    }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    FooterComponent() = default;
    ~FooterComponent() override = default;

    //==============================================================================
    // Accessors
    //==============================================================================

    /**
     * @brief Get bounds of branding text (for centering tests)
     */
    juce::Rectangle<int> getBrandingTextBounds() const
    {
        return brandingBounds;
    }

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Draw background
        g.setColour(getBackgroundColor());
        g.fillRect(bounds);

        // Draw top border
        g.setColour(ReOrderColors::border);
        g.drawHorizontalLine(0, bounds.getX(), bounds.getRight());

        // Draw branding text
        drawBranding(g);
    }

    void resized() override
    {
        // Calculate branding bounds (centered)
        int textWidth = 140;  // Approximate width for "REORDER AUDIO"
        int textHeight = 20;
        int x = (getWidth() - textWidth) / 2;
        int y = (getHeight() - textHeight) / 2;

        brandingBounds = juce::Rectangle<int>(x, y, textWidth, textHeight);
    }

private:
    void drawBranding(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        // Calculate text positioning
        juce::Font font(juce::FontOptions(11.0f, juce::Font::bold));
        g.setFont(font);

        juce::String reorderText = "REORDER ";
        juce::String audioText = "AUDIO";

        float reorderWidth = font.getStringWidthFloat(reorderText);
        float audioWidth = font.getStringWidthFloat(audioText);
        float totalWidth = reorderWidth + audioWidth;

        float startX = (bounds.getWidth() - totalWidth) / 2.0f;
        float textY = (bounds.getHeight() - font.getHeight()) / 2.0f;

        // Draw "REORDER" in muted gray
        g.setColour(getReorderTextColor());
        g.drawText(reorderText, static_cast<int>(startX), static_cast<int>(textY),
                   static_cast<int>(reorderWidth), static_cast<int>(font.getHeight()),
                   juce::Justification::left, false);

        // Draw "AUDIO" in cyan
        g.setColour(getAudioTextColor());
        g.drawText(audioText, static_cast<int>(startX + reorderWidth), static_cast<int>(textY),
                   static_cast<int>(audioWidth), static_cast<int>(font.getHeight()),
                   juce::Justification::left, false);
    }

    //==============================================================================
    // Members
    //==============================================================================

    juce::Rectangle<int> brandingBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FooterComponent)
};
