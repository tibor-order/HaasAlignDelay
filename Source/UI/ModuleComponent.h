/*
  ==============================================================================

    ModuleComponent.h
    Base component for all module panels

    Features:
    - Standard header: power button, title (uppercase), optional badge
    - Icon area: 60-80px for module icon
    - Controls area: flexible layout for knobs/sliders
    - Accent color property
    - Active/inactive state with opacity reduction (30-40% when inactive)
    - Background: #141414, border: 1px #2a2a2a, radius: 10px
    - Active glow border when enabled

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"

/**
 * @brief Base component for all module panels
 *
 * Provides consistent layout and styling for Delay, Width, Phase, and Output modules.
 */
class ModuleComponent : public juce::Component
{
public:
    //==============================================================================
    // Layout Constants
    //==============================================================================

    static constexpr float BORDER_RADIUS = 10.0f;
    static constexpr float HEADER_HEIGHT = 40.0f;
    static constexpr float ICON_AREA_HEIGHT = 80.0f;
    static constexpr float BORDER_WIDTH = 1.0f;
    static constexpr float POWER_BUTTON_SIZE = 20.0f;
    static constexpr float HEADER_PADDING = 10.0f;
    static constexpr float INACTIVE_OPACITY = 0.35f;
    static constexpr float GLOW_RADIUS = 30.0f;

    //==============================================================================
    // Static Accessors
    //==============================================================================

    /**
     * @brief Get background color (#141414)
     */
    static juce::Colour getBackgroundColor()
    {
        return ReOrderColors::bgModule;
    }

    /**
     * @brief Get border color (#2a2a2a)
     */
    static juce::Colour getBorderColor()
    {
        return ReOrderColors::border;
    }

    /**
     * @brief Get border radius (10px)
     */
    static float getBorderRadius()
    {
        return BORDER_RADIUS;
    }

    /**
     * @brief Get header height (40px)
     */
    static float getHeaderHeight()
    {
        return HEADER_HEIGHT;
    }

    /**
     * @brief Get inactive opacity (35%)
     */
    static float getInactiveOpacity()
    {
        return INACTIVE_OPACITY;
    }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    /**
     * @brief Create a module component
     * @param title The module title (will be displayed uppercase)
     * @param accent The accent color for this module
     */
    ModuleComponent(const juce::String& title, juce::Colour accent)
        : moduleTitle(title),
          accentColor(accent),
          isActive(true),
          showGlow(false)
    {
        // Create power button
        powerButton = std::make_unique<juce::ToggleButton>();
        powerButton->setToggleState(true, juce::dontSendNotification);
        powerButton->onClick = [this]()
        {
            setActive(powerButton->getToggleState());
        };
        addAndMakeVisible(powerButton.get());
    }

    ~ModuleComponent() override = default;

    //==============================================================================
    // Title
    //==============================================================================

    /**
     * @brief Get the display title (uppercase)
     */
    juce::String getDisplayTitle() const
    {
        return moduleTitle.toUpperCase();
    }

    /**
     * @brief Get the title color (accent color)
     */
    juce::Colour getTitleColor() const
    {
        return accentColor;
    }

    //==============================================================================
    // Accent Color
    //==============================================================================

    /**
     * @brief Get the accent color
     */
    juce::Colour getAccentColor() const
    {
        return accentColor;
    }

    /**
     * @brief Set the accent color
     */
    void setAccentColor(juce::Colour newAccent)
    {
        accentColor = newAccent;
        repaint();
    }

    //==============================================================================
    // Active State
    //==============================================================================

    /**
     * @brief Set the active state
     */
    void setActive(bool shouldBeActive)
    {
        isActive = shouldBeActive;
        showGlow = shouldBeActive;
        repaint();
    }

    /**
     * @brief Check if module is active
     */
    bool getActive() const
    {
        return isActive;
    }

    /**
     * @brief Check if module has active glow
     */
    bool hasActiveGlow() const
    {
        return showGlow;
    }

    //==============================================================================
    // Power Button
    //==============================================================================

    /**
     * @brief Get the power button
     */
    juce::ToggleButton* getPowerButton()
    {
        return powerButton.get();
    }

    //==============================================================================
    // Layout Bounds
    //==============================================================================

    /**
     * @brief Get the header area bounds
     */
    juce::Rectangle<int> getHeaderBounds() const
    {
        return juce::Rectangle<int>(0, 0, getWidth(), static_cast<int>(HEADER_HEIGHT));
    }

    /**
     * @brief Get the icon area bounds
     */
    juce::Rectangle<int> getIconBounds() const
    {
        int y = static_cast<int>(HEADER_HEIGHT);
        int height = static_cast<int>(ICON_AREA_HEIGHT);
        return juce::Rectangle<int>(0, y, getWidth(), height);
    }

    /**
     * @brief Get the controls area bounds
     */
    juce::Rectangle<int> getControlsBounds() const
    {
        int y = static_cast<int>(HEADER_HEIGHT + ICON_AREA_HEIGHT);
        int height = getHeight() - y;
        return juce::Rectangle<int>(0, y, getWidth(), height);
    }

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Apply inactive opacity
        float opacity = isActive ? 1.0f : INACTIVE_OPACITY;

        //==============================================================================
        // Active Glow
        //==============================================================================
        if (showGlow)
        {
            juce::Path glowPath;
            glowPath.addRoundedRectangle(bounds, BORDER_RADIUS);
            g.setColour(accentColor.withAlpha(0.15f));
            g.strokePath(glowPath, juce::PathStrokeType(GLOW_RADIUS * 0.5f));
        }

        //==============================================================================
        // Background
        //==============================================================================
        g.setColour(getBackgroundColor().withMultipliedAlpha(opacity));
        g.fillRoundedRectangle(bounds, BORDER_RADIUS);

        //==============================================================================
        // Border
        //==============================================================================
        juce::Colour borderCol = showGlow ? accentColor.withAlpha(0.6f) : getBorderColor();
        g.setColour(borderCol.withMultipliedAlpha(opacity));
        g.drawRoundedRectangle(bounds.reduced(BORDER_WIDTH * 0.5f), BORDER_RADIUS, BORDER_WIDTH);

        //==============================================================================
        // Header Separator
        //==============================================================================
        g.setColour(getBorderColor().withMultipliedAlpha(opacity));
        g.drawHorizontalLine(static_cast<int>(HEADER_HEIGHT), 0.0f, static_cast<float>(getWidth()));

        //==============================================================================
        // Title
        //==============================================================================
        auto headerBounds = getHeaderBounds().toFloat();
        auto titleBounds = headerBounds.withTrimmedLeft(HEADER_PADDING + POWER_BUTTON_SIZE + 10.0f)
                                       .withTrimmedRight(HEADER_PADDING);

        g.setColour(getTitleColor().withMultipliedAlpha(opacity));
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(getDisplayTitle(), titleBounds, juce::Justification::centred, true);
    }

    void resized() override
    {
        // Position power button in header
        auto headerBounds = getHeaderBounds();
        int buttonY = (headerBounds.getHeight() - static_cast<int>(POWER_BUTTON_SIZE)) / 2;
        powerButton->setBounds(static_cast<int>(HEADER_PADDING),
                               buttonY,
                               static_cast<int>(POWER_BUTTON_SIZE),
                               static_cast<int>(POWER_BUTTON_SIZE));
    }

protected:
    juce::String moduleTitle;
    juce::Colour accentColor;
    bool isActive;
    bool showGlow;

    std::unique_ptr<juce::ToggleButton> powerButton;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleComponent)
};
