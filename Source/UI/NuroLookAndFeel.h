#pragma once

#include <JuceHeader.h>
#include "NuroColors.h"

namespace UI
{

/**
 * @brief NuroLookAndFeel - Waves X-Vox Pro / Nuro Audio Visual Style
 *
 * Premium dark theme with signature cyan glowing knobs, LED-style meters,
 * and neon toggle effects. Matches the Waves/Nuro Audio design language.
 */
class NuroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NuroLookAndFeel();
    ~NuroLookAndFeel() override = default;

    // === ROTARY KNOB: Signature Nuro style with glowing cyan arc ===
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    // === BUTTONS: Neon toggle buttons with LED glow effects ===
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    // === COMBO BOX: Dark styled dropdown ===
    void drawComboBox(juce::Graphics& g, int width, int height,
                      bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH, juce::ComboBox& box) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    // === HELPER: Draw glowing arc for knobs ===
    static void drawGlowingArc(juce::Graphics& g, float centreX, float centreY,
                               float radius, float lineWidth,
                               float startAngle, float endAngle,
                               juce::Colour colour, float glowIntensity = 1.0f);

    // === HELPER: Draw neon glow effect ===
    static void drawNeonGlow(juce::Graphics& g, juce::Rectangle<float> bounds,
                             juce::Colour colour, float radius, float intensity = 1.0f);

private:
    // Fonts
    juce::Font labelFont;
    juce::Font valueFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NuroLookAndFeel)
};

} // namespace UI
