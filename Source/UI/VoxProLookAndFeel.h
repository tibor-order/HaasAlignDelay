#pragma once

#include <JuceHeader.h>

namespace UI
{

/**
 * @brief VoxProLookAndFeel - XVOX Visual Style + Purple Accent
 *
 * Custom LookAndFeel providing dark theme styling with purple accents,
 * matching the XVOX Pro visual style.
 */
class VoxProLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VoxProLookAndFeel();
    ~VoxProLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    // XVOX Pro color palette - public for use in custom painting
    static inline const juce::Colour accentPurple{0xff7B5FFF};
    static inline const juce::Colour accentPink{0xffff6b9d};
    static inline const juce::Colour accentCyan{0xff00d4ff};
    static inline const juce::Colour accentYellow{0xffffd93d};
    static inline const juce::Colour textWhite{0xffe8e8f0};
    static inline const juce::Colour textGray{0xff8888a0};
    static inline const juce::Colour textDark{0xff4a4a5a};
    static inline const juce::Colour meterGreen{0xff00ff88};
    static inline const juce::Colour meterYellow{0xffffd700};
    static inline const juce::Colour meterRed{0xffff4444};
    static inline const juce::Colour backgroundDark{0xff0a0a0e};
    static inline const juce::Colour panelBackground{0xff0e0e12};
    static inline const juce::Colour panelBorder{0xff1e1e25};
};

} // namespace UI
