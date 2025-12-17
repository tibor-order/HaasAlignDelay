/*
  ==============================================================================

    HeaderComponent.h
    Plugin header with logo, preset selector, and bypass button

    Layout:
    ┌─────────────────────────────────────────────────────────────┐
    │  [LOGO]     │  [< PRESET NAME >]  │     [BYPASS]           │
    └─────────────────────────────────────────────────────────────┘

    Features:
    - Height: 48px
    - Background: gradient #222222 to #1a1a1a
    - Logo on left
    - Centered preset selector
    - Bypass button on right (red when bypassed)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ReOrderColors.h"

/**
 * @brief Header component with logo, preset selector, and bypass button
 */
class HeaderComponent : public juce::Component
{
public:
    //==============================================================================
    // Constants
    //==============================================================================

    static constexpr float DEFAULT_HEIGHT = 48.0f;

    //==============================================================================
    // Static Methods
    //==============================================================================

    static float getDefaultHeight() { return DEFAULT_HEIGHT; }

    static std::pair<juce::Colour, juce::Colour> getBackgroundGradientColors()
    {
        return { juce::Colour(0xff222222), ReOrderColors::bgDark };
    }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    HeaderComponent()
        : hasLogoFlag(true),
          hasPresetSelectorFlag(true)
    {
        // Create bypass button
        bypassButton = std::make_unique<juce::TextButton>("BYPASS");
        bypassButton->setClickingTogglesState(true);
        bypassButton->setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(bypassButton.get());

        // Create preset selector
        presetSelector = std::make_unique<juce::ComboBox>("presetSelector");
        presetSelector->addItem("Default", 1);
        presetSelector->addItem("Vocal Doubler", 2);
        presetSelector->addItem("Wide Stereo", 3);
        presetSelector->setSelectedId(1, juce::dontSendNotification);
        addAndMakeVisible(presetSelector.get());

        // Create preset navigation buttons
        prevPresetButton = std::make_unique<juce::TextButton>("<");
        nextPresetButton = std::make_unique<juce::TextButton>(">");
        addAndMakeVisible(prevPresetButton.get());
        addAndMakeVisible(nextPresetButton.get());
    }

    ~HeaderComponent() override = default;

    //==============================================================================
    // Accessors
    //==============================================================================

    bool hasLogo() const { return hasLogoFlag; }
    bool hasPresetSelector() const { return hasPresetSelectorFlag; }

    juce::Button* getBypassButton() { return bypassButton.get(); }

    /**
     * @brief Get bypass button color based on state
     * @param isBypassed Whether plugin is bypassed
     * @return Color for bypass button
     */
    juce::Colour getBypassButtonColor(bool isBypassed) const
    {
        if (isBypassed)
            return ReOrderColors::statusError;  // Red when bypassed
        return juce::Colour(0xff555555);  // Gray when active
    }

    /**
     * @brief Get bounds of preset selector (for centering tests)
     */
    juce::Rectangle<int> getPresetSelectorBounds() const
    {
        return presetSelectorBounds;
    }

    //==============================================================================
    // Component Overrides
    //==============================================================================

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Draw gradient background
        auto [topColor, bottomColor] = getBackgroundGradientColors();
        juce::ColourGradient gradient(topColor, 0.0f, 0.0f,
                                       bottomColor, 0.0f, bounds.getHeight(),
                                       false);
        g.setGradientFill(gradient);
        g.fillRect(bounds);

        // Draw bottom border
        g.setColour(ReOrderColors::border);
        g.drawHorizontalLine(static_cast<int>(bounds.getBottom()) - 1,
                            bounds.getX(), bounds.getRight());

        // Draw logo
        drawLogo(g);

        // Update bypass button appearance
        bool isBypassed = bypassButton->getToggleState();
        bypassButton->setColour(juce::TextButton::buttonColourId,
                                getBypassButtonColor(isBypassed));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        int padding = 15;

        // Bypass button on right
        int buttonWidth = 70;
        int buttonHeight = 28;
        bypassButton->setBounds(bounds.getRight() - buttonWidth - padding,
                                (bounds.getHeight() - buttonHeight) / 2,
                                buttonWidth, buttonHeight);

        // Preset selector centered
        int presetWidth = 180;
        int presetHeight = 28;
        int arrowWidth = 24;

        int presetAreaWidth = arrowWidth + presetWidth + arrowWidth;
        int presetX = (bounds.getWidth() - presetAreaWidth) / 2;
        int presetY = (bounds.getHeight() - presetHeight) / 2;

        prevPresetButton->setBounds(presetX, presetY, arrowWidth, presetHeight);
        presetSelector->setBounds(presetX + arrowWidth, presetY, presetWidth, presetHeight);
        nextPresetButton->setBounds(presetX + arrowWidth + presetWidth, presetY, arrowWidth, presetHeight);

        // Store preset selector bounds for testing
        presetSelectorBounds = juce::Rectangle<int>(presetX, presetY, presetAreaWidth, presetHeight);
    }

private:
    void drawLogo(juce::Graphics& g)
    {
        auto bounds = getLocalBounds();
        int logoX = 15;
        int logoY = (bounds.getHeight() - 24) / 2;

        // Draw "HAAS FX" text as logo placeholder
        g.setColour(ReOrderColors::accentWidth);
        g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        g.drawText("HAAS FX", logoX, logoY, 80, 24, juce::Justification::centredLeft, false);

        // "PRO" badge
        g.setColour(ReOrderColors::accentOutput);
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText("PRO", logoX + 70, logoY + 2, 30, 20, juce::Justification::centredLeft, false);
    }

    //==============================================================================
    // Members
    //==============================================================================

    bool hasLogoFlag;
    bool hasPresetSelectorFlag;

    std::unique_ptr<juce::TextButton> bypassButton;
    std::unique_ptr<juce::ComboBox> presetSelector;
    std::unique_ptr<juce::TextButton> prevPresetButton;
    std::unique_ptr<juce::TextButton> nextPresetButton;

    juce::Rectangle<int> presetSelectorBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderComponent)
};
