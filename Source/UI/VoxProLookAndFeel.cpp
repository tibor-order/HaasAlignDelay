#include "VoxProLookAndFeel.h"

namespace UI
{

VoxProLookAndFeel::VoxProLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, textWhite);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void VoxProLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Scale factor for drawing details
    float scale = radius / 42.0f;

    // === XVOX PRO STYLE: Large chunky matte knob ===
    auto knobRadius = radius * 0.82f;

    // === GLOWING PURPLE TICK MARKS AROUND KNOB PERIMETER ===
    float tickRadius = knobRadius + 8 * scale;
    int numTicks = 13;
    float tickLength = 6.0f * scale;
    float tickWidth = 2.5f * scale;

    for (int i = 0; i <= numTicks; ++i)
    {
        float tickAngle = rotaryStartAngle + (float(i) / numTicks) * (rotaryEndAngle - rotaryStartAngle);
        float tickInner = tickRadius;
        float tickOuter = tickRadius + tickLength;

        float tx1 = centreX + std::sin(tickAngle) * tickInner;
        float ty1 = centreY - std::cos(tickAngle) * tickInner;
        float tx2 = centreX + std::sin(tickAngle) * tickOuter;
        float ty2 = centreY - std::cos(tickAngle) * tickOuter;

        float tickPos = float(i) / numTicks;
        bool isLit = tickPos <= sliderPos;

        if (isLit)
        {
            // Outer glow layer (soft bloom effect)
            g.setColour(accentPurple.withAlpha(0.15f));
            g.drawLine(tx1, ty1, tx2, ty2, tickWidth * 4.0f);

            // Middle glow layer
            g.setColour(accentPurple.withAlpha(0.3f));
            g.drawLine(tx1, ty1, tx2, ty2, tickWidth * 2.5f);

            // Inner glow layer
            g.setColour(accentPurple.withAlpha(0.5f));
            g.drawLine(tx1, ty1, tx2, ty2, tickWidth * 1.5f);

            // Core bright tick
            g.setColour(juce::Colour(0xff9b7fff));
            g.drawLine(tx1, ty1, tx2, ty2, tickWidth);
        }
        else
        {
            g.setColour(juce::Colour(0xff2a2a32));
            g.drawLine(tx1, ty1, tx2, ty2, tickWidth * 0.8f);
        }
    }

    // === SOFT DROP SHADOW - Creates lifted/embossed 3D appearance ===
    float shadowOffsetX = 0.0f;
    float shadowOffsetY = 3.0f * scale;

    // Outermost shadow layer (~10px outward)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.08f));
    g.fillEllipse(centreX - knobRadius - 10 * scale + shadowOffsetX,
                  centreY - knobRadius - 10 * scale + shadowOffsetY + 4 * scale,
                  (knobRadius + 10 * scale) * 2.0f, (knobRadius + 10 * scale) * 2.0f);

    // Second shadow layer (~7px outward)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.12f));
    g.fillEllipse(centreX - knobRadius - 7 * scale + shadowOffsetX,
                  centreY - knobRadius - 7 * scale + shadowOffsetY + 3 * scale,
                  (knobRadius + 7 * scale) * 2.0f, (knobRadius + 7 * scale) * 2.0f);

    // Third shadow layer (~5px outward)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.18f));
    g.fillEllipse(centreX - knobRadius - 5 * scale + shadowOffsetX,
                  centreY - knobRadius - 5 * scale + shadowOffsetY + 2 * scale,
                  (knobRadius + 5 * scale) * 2.0f, (knobRadius + 5 * scale) * 2.0f);

    // Inner shadow layer (closest to knob edge)
    g.setColour(juce::Colour(0xff050508).withAlpha(0.3f));
    g.fillEllipse(centreX - knobRadius - 3 * scale + shadowOffsetX,
                  centreY - knobRadius - 3 * scale + shadowOffsetY + 1 * scale,
                  (knobRadius + 3 * scale) * 2.0f, (knobRadius + 3 * scale) * 2.0f);

    // Outer ring - dark beveled edge
    g.setColour(juce::Colour(0xff1a1a20));
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    // Main knob body - flat matte dark gray
    auto bodyRadius = knobRadius - 3.0f * scale;

    juce::ColourGradient knobGradient(
        juce::Colour(0xff2e2e36), centreX, centreY,
        juce::Colour(0xff222228), centreX, centreY + bodyRadius, true);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centreX - bodyRadius, centreY - bodyRadius,
                  bodyRadius * 2.0f, bodyRadius * 2.0f);

    // Inner recessed circle
    auto innerRadius = bodyRadius * 0.75f;
    g.setColour(juce::Colour(0xff1c1c22));
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f);

    // Subtle inner highlight ring
    g.setColour(juce::Colour(0xff28282f));
    g.drawEllipse(centreX - innerRadius, centreY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f, 1.5f * scale);

    // === White indicator notch near edge ===
    float indicatorInner = bodyRadius * 0.72f;
    float indicatorOuter = bodyRadius - 4.0f * scale;

    float innerX = centreX + std::sin(angle) * indicatorInner;
    float innerY = centreY - std::cos(angle) * indicatorInner;
    float outerX = centreX + std::sin(angle) * indicatorOuter;
    float outerY = centreY - std::cos(angle) * indicatorOuter;

    g.setColour(juce::Colours::white.withAlpha(0.95f));
    g.drawLine(innerX, innerY, outerX, outerY, 2.0f * scale);
}

void VoxProLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour&,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    float scale = bounds.getHeight() / 24.0f;
    float cornerSize = 5.0f * scale;
    float strokeWidth = 1.0f * scale;

    bool isOn = button.getToggleState();
    juce::String buttonText = button.getButtonText();
    bool isBypass = buttonText == "BYPASS";
    bool isPower = buttonText.isEmpty();
    bool isLink = buttonText == "~";
    bool isAutoPhase = buttonText == "AUTO";
    bool isScaleButton = buttonText.endsWith("%");

    if (isPower || isLink)
        return;

    if (isScaleButton)
    {
        // Scale button - subtle dark style with purple accent
        g.setColour(juce::Colour(0xff0f0f14));
        g.fillRoundedRectangle(bounds, cornerSize);

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour(accentPurple.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, cornerSize);
        }

        g.setColour(accentPurple.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds, cornerSize, strokeWidth);
    }
    else if (isBypass)
    {
        g.setColour(juce::Colour(0xff0f0f14));
        g.fillRoundedRectangle(bounds, cornerSize);

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour(juce::Colours::white.withAlpha(0.02f));
            g.fillRoundedRectangle(bounds, cornerSize);
        }

        g.setColour(juce::Colour(0xff2a2a35));
        g.drawRoundedRectangle(bounds, cornerSize, strokeWidth);
    }
    else if (isAutoPhase)
    {
        if (isOn)
        {
            g.setColour(accentYellow.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds.expanded(2 * scale), cornerSize + 1 * scale);
            g.setColour(accentYellow.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, cornerSize);
            g.setColour(accentYellow.withAlpha(0.6f));
            g.drawRoundedRectangle(bounds, cornerSize, strokeWidth);
        }
        else
        {
            g.setColour(juce::Colour(0xff0f0f14));
            g.fillRoundedRectangle(bounds, cornerSize);
            g.setColour(juce::Colour(0xff2a2a35));
            g.drawRoundedRectangle(bounds, cornerSize, strokeWidth);
        }
    }
    else
    {
        // Phase buttons - purple when on
        if (isOn)
        {
            g.setColour(accentPurple.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds.expanded(2 * scale), cornerSize + 1 * scale);
            g.setColour(accentPurple.withAlpha(0.25f));
            g.fillRoundedRectangle(bounds, cornerSize);
            g.setColour(accentPurple.withAlpha(0.6f));
            g.drawRoundedRectangle(bounds, cornerSize, strokeWidth);
        }
        else
        {
            g.setColour(juce::Colour(0xff0f0f14));
            g.fillRoundedRectangle(bounds, cornerSize);
            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(juce::Colours::white.withAlpha(0.02f));
                g.fillRoundedRectangle(bounds, cornerSize);
            }
            g.setColour(juce::Colour(0xff2a2a35));
            g.drawRoundedRectangle(bounds, cornerSize, strokeWidth);
        }
    }
}

void VoxProLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                        bool, bool)
{
    juce::String buttonText = button.getButtonText();
    auto bounds = button.getLocalBounds().toFloat();
    float scale = bounds.getHeight() / 24.0f;

    if (buttonText.isEmpty())
    {
        bool isOn = button.getToggleState();
        g.setColour(isOn ? accentPink : textGray);

        auto iconBounds = bounds.reduced(4 * scale);
        float cx = iconBounds.getCentreX();
        float cy = iconBounds.getCentreY();
        float iconRadius = juce::jmin(iconBounds.getWidth(), iconBounds.getHeight()) / 2.0f - 2 * scale;

        juce::Path arc;
        arc.addCentredArc(cx, cy, iconRadius, iconRadius, 0,
                          juce::MathConstants<float>::pi * 0.3f,
                          juce::MathConstants<float>::pi * 1.7f, true);
        g.strokePath(arc, juce::PathStrokeType(1.5f * scale));
        g.drawLine(cx, cy - iconRadius - 1 * scale, cx, cy - 2 * scale, 1.5f * scale);
        return;
    }

    bool isOn = button.getToggleState();
    bool isLink = buttonText == "~";
    bool isAutoPhase = buttonText == "AUTO";
    bool isScaleButton = buttonText.endsWith("%");

    if (isLink)
    {
        g.setColour(isOn ? accentPurple : textGray);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float linkSize = 5.0f * scale;
        float gap = 2.0f * scale;

        g.drawEllipse(cx - linkSize * 2 - gap, cy - linkSize, linkSize * 2, linkSize * 2, 1.5f * scale);
        g.drawEllipse(cx + gap, cy - linkSize, linkSize * 2, linkSize * 2, 1.5f * scale);

        if (isOn)
        {
            g.drawLine(cx - gap, cy - 2 * scale, cx + gap, cy - 2 * scale, 1.5f * scale);
            g.drawLine(cx - gap, cy + 2 * scale, cx + gap, cy + 2 * scale, 1.5f * scale);
        }
        return;
    }

    // Text colors
    if (isAutoPhase)
        g.setColour(isOn ? juce::Colour(0xff101015) : textGray);
    else if (isScaleButton)
        g.setColour(accentPurple);
    else
        g.setColour(isOn ? juce::Colours::white : textGray);

    float fontSize = bounds.getHeight() * 0.5f;
    g.setFont(juce::FontOptions(fontSize).withStyle("Bold"));
    g.drawText(buttonText, bounds, juce::Justification::centred);
}

} // namespace UI
