#include "NuroLookAndFeel.h"

namespace UI
{

NuroLookAndFeel::NuroLookAndFeel()
{
    // Set default colours
    setColour(juce::Slider::textBoxTextColourId, NuroColors::colour(NuroColors::textWhite));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::ComboBox::backgroundColourId, NuroColors::colour(NuroColors::panelBackground));
    setColour(juce::ComboBox::outlineColourId, NuroColors::colour(NuroColors::borderMedium));
    setColour(juce::ComboBox::textColourId, NuroColors::colour(NuroColors::textWhite));
    setColour(juce::ComboBox::arrowColourId, NuroColors::colour(NuroColors::accentCyan));

    setColour(juce::PopupMenu::backgroundColourId, NuroColors::colour(NuroColors::panelBackground));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, NuroColors::colour(NuroColors::accentCyan).withAlpha(0.2f));
    setColour(juce::PopupMenu::textColourId, NuroColors::colour(NuroColors::textWhite));
    setColour(juce::PopupMenu::highlightedTextColourId, NuroColors::colour(NuroColors::textWhite));
}

void NuroLookAndFeel::drawGlowingArc(juce::Graphics& g, float centreX, float centreY,
                                      float radius, float lineWidth,
                                      float startAngle, float endAngle,
                                      juce::Colour colour, float glowIntensity)
{
    if (startAngle >= endAngle)
        return;

    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius, radius, 0.0f, startAngle, endAngle, true);

    // === OUTER BLOOM GLOW (largest, most diffuse) ===
    g.setColour(colour.withAlpha(0.08f * glowIntensity));
    g.strokePath(arc, juce::PathStrokeType(lineWidth + 20.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // === MEDIUM GLOW ===
    g.setColour(colour.withAlpha(0.15f * glowIntensity));
    g.strokePath(arc, juce::PathStrokeType(lineWidth + 12.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // === INNER GLOW ===
    g.setColour(colour.withAlpha(0.3f * glowIntensity));
    g.strokePath(arc, juce::PathStrokeType(lineWidth + 6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // === CORE ARC (bright center) ===
    g.setColour(colour);
    g.strokePath(arc, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // === HIGHLIGHT (white-ish core for extra brightness) ===
    g.setColour(colour.brighter(0.3f).withAlpha(0.8f));
    g.strokePath(arc, juce::PathStrokeType(lineWidth * 0.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void NuroLookAndFeel::drawNeonGlow(juce::Graphics& g, juce::Rectangle<float> bounds,
                                    juce::Colour colour, float radius, float intensity)
{
    // Multiple layers for realistic glow
    for (int i = 4; i >= 0; --i)
    {
        float expand = static_cast<float>(i) * radius * 0.3f;
        float alpha = 0.1f * intensity * (1.0f - static_cast<float>(i) * 0.15f);
        g.setColour(colour.withAlpha(alpha));
        g.fillRoundedRectangle(bounds.expanded(expand), 6.0f + expand * 0.5f);
    }
}

void NuroLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    float scale = radius / 42.0f;

    // === KNOB SIZING ===
    auto knobRadius = radius * 0.72f;      // Main knob body
    auto arcRadius = radius * 0.88f;        // Arc indicator radius
    float arcWidth = 3.5f * scale;          // Arc line thickness

    // === DROP SHADOW (creates depth) ===
    for (int i = 4; i >= 1; --i)
    {
        float shadowOffset = i * 1.5f * scale;
        float shadowAlpha = 0.06f * (5 - i);
        g.setColour(juce::Colours::black.withAlpha(shadowAlpha));
        g.fillEllipse(centreX - knobRadius - shadowOffset,
                      centreY - knobRadius + shadowOffset * 0.5f,
                      (knobRadius + shadowOffset) * 2.0f,
                      (knobRadius + shadowOffset) * 2.0f);
    }

    // === BACKGROUND ARC TRACK (unlit portion) ===
    {
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(NuroColors::colour(NuroColors::knobInner));
        g.strokePath(bgArc, juce::PathStrokeType(arcWidth + 2.0f * scale,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // === GLOWING CYAN ARC (the signature Nuro look!) ===
    auto accentColour = NuroColors::colour(NuroColors::accentCyan);

    // Determine if this is the width slider for special coloring when correction active
    bool isHovered = slider.isMouseOver();
    float glowIntensity = isHovered ? 1.3f : 1.0f;

    drawGlowingArc(g, centreX, centreY, arcRadius, arcWidth,
                   rotaryStartAngle, angle, accentColour, glowIntensity);

    // === KNOB BASE (outer ring) ===
    g.setColour(NuroColors::colour(NuroColors::knobInner));
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    // === KNOB BODY (main surface with subtle gradient) ===
    auto bodyRadius = knobRadius - 2.0f * scale;
    juce::ColourGradient knobGradient(
        NuroColors::colour(NuroColors::knobSurface), centreX, centreY - bodyRadius * 0.5f,
        NuroColors::colour(NuroColors::knobSurfaceEdge), centreX, centreY + bodyRadius, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centreX - bodyRadius, centreY - bodyRadius,
                  bodyRadius * 2.0f, bodyRadius * 2.0f);

    // === INNER RECESSED CIRCLE ===
    auto innerRadius = bodyRadius * 0.65f;
    g.setColour(NuroColors::colour(NuroColors::knobInner));
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f);

    // Subtle inner ring
    g.setColour(NuroColors::colour(NuroColors::knobBase).brighter(0.05f));
    g.drawEllipse(centreX - innerRadius + 1, centreY - innerRadius + 1,
                  innerRadius * 2.0f - 2, innerRadius * 2.0f - 2, 1.0f * scale);

    // === INDICATOR DOT ON ARC ===
    {
        float dotRadius = 5.0f * scale;
        float dotX = centreX + std::sin(angle) * arcRadius;
        float dotY = centreY - std::cos(angle) * arcRadius;

        // Glow behind dot
        g.setColour(accentColour.withAlpha(0.4f * glowIntensity));
        g.fillEllipse(dotX - dotRadius * 2, dotY - dotRadius * 2, dotRadius * 4, dotRadius * 4);

        // White dot
        g.setColour(juce::Colours::white);
        g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);
    }

    // === WHITE INDICATOR LINE ON KNOB ===
    {
        float indicatorInner = innerRadius * 0.4f;
        float indicatorOuter = bodyRadius - 3.0f * scale;

        float innerX = centreX + std::sin(angle) * indicatorInner;
        float innerY = centreY - std::cos(angle) * indicatorInner;
        float outerX = centreX + std::sin(angle) * indicatorOuter;
        float outerY = centreY - std::cos(angle) * indicatorOuter;

        // Subtle glow behind line
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawLine(innerX, innerY, outerX, outerY, 4.0f * scale);

        // White line
        g.setColour(juce::Colours::white.withAlpha(0.95f));
        g.drawLine(innerX, innerY, outerX, outerY, 2.0f * scale);
    }
}

void NuroLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    float scale = bounds.getHeight() / 24.0f;
    float cornerSize = 4.0f * scale;

    bool isOn = button.getToggleState();
    juce::String buttonText = button.getButtonText();
    bool isBypass = buttonText == "BYPASS";
    bool isAutoPhase = buttonText == "AUTO";
    bool isScaleButton = buttonText.endsWith("%");

    auto accentColour = NuroColors::colour(NuroColors::accentCyan);

    if (isScaleButton)
    {
        // Scale button - subtle dark style
        g.setColour(NuroColors::colour(NuroColors::panelBackground));
        g.fillRoundedRectangle(bounds, cornerSize);

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour(accentColour.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, cornerSize);
        }

        g.setColour(NuroColors::colour(NuroColors::borderMedium));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }
    else if (isBypass)
    {
        // Bypass button - red glow when active
        if (isOn)
        {
            // Outer glow
            drawNeonGlow(g, bounds, NuroColors::colour(NuroColors::meterRed), 8.0f, 0.5f);

            g.setColour(NuroColors::colour(NuroColors::meterRed).withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, cornerSize);

            g.setColour(NuroColors::colour(NuroColors::meterRed).withAlpha(0.8f));
            g.drawRoundedRectangle(bounds, cornerSize, 1.5f * scale);
        }
        else
        {
            g.setColour(NuroColors::colour(NuroColors::panelBackground));
            g.fillRoundedRectangle(bounds, cornerSize);

            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(juce::Colours::white.withAlpha(0.03f));
                g.fillRoundedRectangle(bounds, cornerSize);
            }

            g.setColour(NuroColors::colour(NuroColors::borderMedium));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
    }
    else if (isAutoPhase)
    {
        // Auto Phase button - yellow/cyan glow when active
        auto activeColour = NuroColors::colour(NuroColors::accentYellow);

        if (isOn)
        {
            // Outer glow
            drawNeonGlow(g, bounds, activeColour, 10.0f, 0.6f);

            g.setColour(activeColour.withAlpha(0.25f));
            g.fillRoundedRectangle(bounds, cornerSize);

            g.setColour(activeColour);
            g.drawRoundedRectangle(bounds, cornerSize, 1.5f * scale);
        }
        else
        {
            g.setColour(NuroColors::colour(NuroColors::panelBackground));
            g.fillRoundedRectangle(bounds, cornerSize);

            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(activeColour.withAlpha(0.05f));
                g.fillRoundedRectangle(bounds, cornerSize);
            }

            g.setColour(NuroColors::colour(NuroColors::borderMedium));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
    }
    else
    {
        // Phase buttons - cyan glow when on
        if (isOn)
        {
            // Outer glow
            drawNeonGlow(g, bounds, accentColour, 8.0f, 0.5f);

            g.setColour(accentColour.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, cornerSize);

            g.setColour(accentColour);
            g.drawRoundedRectangle(bounds, cornerSize, 1.5f * scale);
        }
        else
        {
            g.setColour(NuroColors::colour(NuroColors::panelBackground));
            g.fillRoundedRectangle(bounds, cornerSize);

            if (shouldDrawButtonAsHighlighted)
            {
                g.setColour(accentColour.withAlpha(0.05f));
                g.fillRoundedRectangle(bounds, cornerSize);
            }

            g.setColour(NuroColors::colour(NuroColors::borderMedium));
            g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        }
    }
}

void NuroLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    juce::String buttonText = button.getButtonText();
    auto bounds = button.getLocalBounds().toFloat();
    float scale = bounds.getHeight() / 24.0f;

    bool isOn = button.getToggleState();
    bool isAutoPhase = buttonText == "AUTO";
    bool isBypass = buttonText == "BYPASS";
    bool isScaleButton = buttonText.endsWith("%");

    // Text colors based on state
    if (isAutoPhase)
    {
        if (isOn)
            g.setColour(NuroColors::colour(NuroColors::accentYellow));
        else
            g.setColour(NuroColors::colour(NuroColors::textDisabled));
    }
    else if (isBypass)
    {
        if (isOn)
            g.setColour(NuroColors::colour(NuroColors::meterRed));
        else
            g.setColour(NuroColors::colour(NuroColors::textDisabled));
    }
    else if (isScaleButton)
    {
        g.setColour(NuroColors::colour(NuroColors::accentCyan));
    }
    else
    {
        if (isOn)
            g.setColour(NuroColors::colour(NuroColors::accentCyan));
        else
            g.setColour(NuroColors::colour(NuroColors::textDisabled));
    }

    float fontSize = bounds.getHeight() * 0.5f;
    g.setFont(juce::FontOptions(fontSize).withStyle("Bold"));
    g.drawText(buttonText, bounds, juce::Justification::centred);
}

void NuroLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                    bool isButtonDown, int, int, int, int,
                                    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1);
    float cornerSize = 4.0f;

    // Background
    g.setColour(NuroColors::colour(NuroColors::panelBackground));
    g.fillRoundedRectangle(bounds, cornerSize);

    // Border with cyan accent
    auto borderColour = box.isMouseOver() ?
        NuroColors::colour(NuroColors::accentCyan).withAlpha(0.5f) :
        NuroColors::colour(NuroColors::borderMedium);
    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

    // Arrow
    auto arrowArea = bounds.removeFromRight(bounds.getHeight()).reduced(8);
    juce::Path arrow;
    arrow.addTriangle(arrowArea.getX(), arrowArea.getCentreY() - 3,
                      arrowArea.getRight(), arrowArea.getCentreY() - 3,
                      arrowArea.getCentreX(), arrowArea.getCentreY() + 3);
    g.setColour(NuroColors::colour(NuroColors::accentCyan));
    g.fillPath(arrow);
}

void NuroLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool isSeparator, bool isActive, bool isHighlighted,
                                         bool isTicked, bool,
                                         const juce::String& text, const juce::String&,
                                         const juce::Drawable*, const juce::Colour*)
{
    auto bounds = area.toFloat().reduced(2, 1);

    if (isSeparator)
    {
        g.setColour(NuroColors::colour(NuroColors::borderMedium));
        g.fillRect(bounds.reduced(10, bounds.getHeight() / 2 - 0.5f).withHeight(1));
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour(NuroColors::colour(NuroColors::accentCyan).withAlpha(0.15f));
        g.fillRoundedRectangle(bounds, 4.0f);
    }

    // Tick mark
    if (isTicked)
    {
        auto tickArea = bounds.removeFromLeft(bounds.getHeight()).reduced(6);
        g.setColour(NuroColors::colour(NuroColors::accentCyan));
        g.fillEllipse(tickArea.getCentreX() - 3, tickArea.getCentreY() - 3, 6, 6);
    }

    // Text
    g.setColour(isActive ? NuroColors::colour(NuroColors::textWhite)
                         : NuroColors::colour(NuroColors::textDisabled));
    g.setFont(juce::FontOptions(14.0f));
    g.drawText(text, bounds.reduced(8, 0), juce::Justification::centredLeft);
}

} // namespace UI
