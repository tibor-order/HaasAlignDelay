/*
  ==============================================================================

    ReOrderLookAndFeel.h
    Custom LookAndFeel for ReOrder Audio plugins

    Features:
    - Custom rotary slider (knob) rendering
    - 270° rotation range (-135° to +135°)
    - Multi-layer glow effect on value arc
    - 3D metallic knob body (radial gradient)
    - Center cap (35% of knob size)
    - Indicator line on knob + dot on arc
    - Drop shadow and arc glow effects

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../ReOrderColors.h"

/**
 * @brief Shadow parameters structure
 */
struct ShadowParams
{
    int offsetX = 0;
    int offsetY = 4;
    int blurRadius = 12;
    float alpha = 0.5f;
};

/**
 * @brief Custom LookAndFeel for ReOrder Audio plugins
 */
class ReOrderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==============================================================================
    // Rotary Slider Constants
    //==============================================================================

    static constexpr float ROTARY_START_ANGLE_DEG = -135.0f;
    static constexpr float ROTARY_END_ANGLE_DEG = 135.0f;
    static constexpr float ROTARY_TOTAL_ANGLE_DEG = 270.0f;

    static constexpr float CENTER_CAP_PERCENTAGE = 0.35f;
    static constexpr float INDICATOR_WIDTH = 2.0f;
    static constexpr float BORDER_WIDTH = 2.0f;
    static constexpr float ARC_GLOW_RADIUS = 20.0f;

    //==============================================================================
    // Linear Slider Constants
    //==============================================================================

    static constexpr float VERTICAL_TRACK_WIDTH = 12.0f;
    static constexpr float VERTICAL_FILL_WIDTH = 6.0f;
    static constexpr float VERTICAL_HANDLE_WIDTH = 20.0f;
    static constexpr float VERTICAL_HANDLE_HEIGHT = 16.0f;
    static constexpr float HORIZONTAL_TRACK_HEIGHT = 8.0f;
    static constexpr float HORIZONTAL_THUMB_SIZE = 16.0f;
    static constexpr float LINEAR_SLIDER_CORNER_RADIUS = 6.0f;
    static constexpr float LINEAR_SLIDER_FILL_GLOW_RADIUS = 10.0f;

    //==============================================================================
    // Button Constants
    //==============================================================================

    static constexpr float POWER_BUTTON_SIZE = 20.0f;
    static constexpr float POWER_BUTTON_BORDER_WIDTH = 2.0f;
    static constexpr float TOGGLE_BUTTON_BORDER_RADIUS = 5.0f;
    static constexpr float TOGGLE_BUTTON_GLOW_RADIUS = 20.0f;
    static constexpr float BYPASS_BUTTON_GLOW_RADIUS = 15.0f;

    //==============================================================================
    // Static Accessors for Testing
    //==============================================================================

    /**
     * @brief Get total rotation angle in degrees
     */
    static float getRotaryTotalAngle()
    {
        return ROTARY_TOTAL_ANGLE_DEG;
    }

    /**
     * @brief Get start angle in radians
     */
    static float getRotaryStartAngle()
    {
        return ROTARY_START_ANGLE_DEG * juce::MathConstants<float>::pi / 180.0f;
    }

    /**
     * @brief Get end angle in radians
     */
    static float getRotaryEndAngle()
    {
        return ROTARY_END_ANGLE_DEG * juce::MathConstants<float>::pi / 180.0f;
    }

    /**
     * @brief Convert normalized value (0-1) to angle in radians
     */
    static float valueToAngle(float normalizedValue)
    {
        float startRad = getRotaryStartAngle();
        float endRad = getRotaryEndAngle();
        return startRad + normalizedValue * (endRad - startRad);
    }

    /**
     * @brief Get knob body gradient colors
     */
    static std::pair<juce::Colour, juce::Colour> getKnobBodyGradientColors()
    {
        return { juce::Colour(0xff3a3a3a), juce::Colour(0xff1a1a1a) };
    }

    /**
     * @brief Get center cap size as percentage of knob diameter
     */
    static float getCenterCapSizePercentage()
    {
        return CENTER_CAP_PERCENTAGE;
    }

    /**
     * @brief Get indicator line width
     */
    static float getIndicatorWidth()
    {
        return INDICATOR_WIDTH;
    }

    /**
     * @brief Get default accent color (used when no property is set)
     */
    static juce::Colour getDefaultAccentColor()
    {
        return ReOrderColors::accentDelay;  // Default to pink
    }

    /**
     * @brief Get arc glow radius
     */
    static float getArcGlowRadius()
    {
        return ARC_GLOW_RADIUS;
    }

    /**
     * @brief Get knob drop shadow parameters
     */
    static ShadowParams getKnobDropShadow()
    {
        return { 0, 4, 12, 0.5f };
    }

    /**
     * @brief Get knob border color
     */
    static juce::Colour getKnobBorderColor()
    {
        return juce::Colour(0xff333333);
    }

    //==============================================================================
    // Linear Slider Static Accessors
    //==============================================================================

    /**
     * @brief Get vertical slider track width
     */
    static float getVerticalSliderTrackWidth()
    {
        return VERTICAL_TRACK_WIDTH;
    }

    /**
     * @brief Get vertical slider fill width
     */
    static float getVerticalSliderFillWidth()
    {
        return VERTICAL_FILL_WIDTH;
    }

    /**
     * @brief Check if vertical slider fill starts from bottom
     */
    static bool isVerticalSliderFillFromBottom()
    {
        return true;
    }

    /**
     * @brief Get vertical slider handle size (width, height)
     */
    static std::pair<float, float> getVerticalSliderHandleSize()
    {
        return { VERTICAL_HANDLE_WIDTH, VERTICAL_HANDLE_HEIGHT };
    }

    /**
     * @brief Get handle gradient colors (top, bottom)
     */
    static std::pair<juce::Colour, juce::Colour> getHandleGradientColors()
    {
        return { juce::Colour(0xff555555), juce::Colour(0xff333333) };
    }

    /**
     * @brief Get linear slider fill glow radius
     */
    static float getLinearSliderFillGlowRadius()
    {
        return LINEAR_SLIDER_FILL_GLOW_RADIUS;
    }

    /**
     * @brief Get horizontal slider track height
     */
    static float getHorizontalSliderTrackHeight()
    {
        return HORIZONTAL_TRACK_HEIGHT;
    }

    /**
     * @brief Check if horizontal slider fill starts from left
     */
    static bool isHorizontalSliderFillFromLeft()
    {
        return true;
    }

    /**
     * @brief Get horizontal slider thumb size
     */
    static float getHorizontalSliderThumbSize()
    {
        return HORIZONTAL_THUMB_SIZE;
    }

    /**
     * @brief Check if linear slider has inset shadow
     */
    static bool hasLinearSliderInsetShadow()
    {
        return true;
    }

    /**
     * @brief Get linear slider corner radius
     */
    static float getLinearSliderCornerRadius()
    {
        return LINEAR_SLIDER_CORNER_RADIUS;
    }

    //==============================================================================
    // Button Static Accessors
    //==============================================================================

    /**
     * @brief Get power button size
     */
    static float getPowerButtonSize()
    {
        return POWER_BUTTON_SIZE;
    }

    /**
     * @brief Get power button off border color
     */
    static juce::Colour getPowerButtonOffBorderColor()
    {
        return juce::Colour(0xff444444);
    }

    /**
     * @brief Get power button on border color (uses accent)
     */
    static juce::Colour getPowerButtonOnBorderColor(juce::Colour accent)
    {
        return accent;
    }

    /**
     * @brief Get power button off fill color (transparent)
     */
    static juce::Colour getPowerButtonOffFillColor()
    {
        return juce::Colours::transparentBlack;
    }

    /**
     * @brief Get power button on fill color (20% accent)
     */
    static juce::Colour getPowerButtonOnFillColor(juce::Colour accent)
    {
        return accent.withAlpha(0.2f);
    }

    /**
     * @brief Get toggle button off background color
     */
    static juce::Colour getToggleButtonOffBackground()
    {
        return juce::Colour(0xff1a1a1a);
    }

    /**
     * @brief Get toggle button off text color
     */
    static juce::Colour getToggleButtonOffTextColor()
    {
        return juce::Colour(0xff666666);
    }

    /**
     * @brief Check if toggle button on state has gradient
     */
    static bool toggleButtonOnHasGradient()
    {
        return true;
    }

    /**
     * @brief Get toggle button on glow radius
     */
    static float getToggleButtonOnGlowRadius()
    {
        return TOGGLE_BUTTON_GLOW_RADIUS;
    }

    /**
     * @brief Get toggle button border radius
     */
    static float getToggleButtonBorderRadius()
    {
        return TOGGLE_BUTTON_BORDER_RADIUS;
    }

    /**
     * @brief Get bypass button active color (gray when processing)
     */
    static juce::Colour getBypassButtonActiveColor()
    {
        return juce::Colour(0xff555555);  // Gray
    }

    /**
     * @brief Get bypass button bypassed color (red)
     */
    static juce::Colour getBypassButtonBypassedColor()
    {
        return ReOrderColors::statusError;  // #ff3366
    }

    /**
     * @brief Get bypass button bypassed glow radius
     */
    static float getBypassButtonBypassedGlowRadius()
    {
        return BYPASS_BUTTON_GLOW_RADIUS;
    }

    //==============================================================================
    // Constructor/Destructor
    //==============================================================================

    ReOrderLookAndFeel()
    {
        // Set default colors for all components
        setColour(juce::Slider::backgroundColourId, ReOrderColors::bgDark);
        setColour(juce::Slider::thumbColourId, ReOrderColors::textPrimary);
        setColour(juce::Slider::trackColourId, ReOrderColors::border);
        setColour(juce::Slider::rotarySliderFillColourId, getDefaultAccentColor());
        setColour(juce::Slider::rotarySliderOutlineColourId, getKnobBorderColor());
    }

    ~ReOrderLookAndFeel() override = default;

    //==============================================================================
    // Rotary Slider Drawing
    //==============================================================================

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();

        // Get accent color from slider property or use default
        juce::Colour accentColor = slider.findColour(juce::Slider::rotarySliderFillColourId);

        // Calculate angle
        float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        //==============================================================================
        // Drop Shadow
        //==============================================================================
        {
            auto shadow = getKnobDropShadow();
            juce::Path shadowPath;
            shadowPath.addEllipse(centreX - radius + shadow.offsetX,
                                  centreY - radius + shadow.offsetY,
                                  radius * 2.0f, radius * 2.0f);
            g.setColour(juce::Colours::black.withAlpha(shadow.alpha));
            g.fillPath(shadowPath);
        }

        //==============================================================================
        // Arc Glow
        //==============================================================================
        {
            juce::Path arcPath;
            float arcRadius = radius - 4.0f;
            arcPath.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                                  0.0f, rotaryStartAngle, angle, true);

            // Outer glow
            g.setColour(accentColor.withAlpha(0.15f));
            g.strokePath(arcPath, juce::PathStrokeType(8.0f));

            // Inner glow
            g.setColour(accentColor.withAlpha(0.3f));
            g.strokePath(arcPath, juce::PathStrokeType(4.0f));

            // Main arc
            g.setColour(accentColor);
            g.strokePath(arcPath, juce::PathStrokeType(2.0f));
        }

        //==============================================================================
        // Knob Body (Radial Gradient)
        //==============================================================================
        {
            auto [innerColor, outerColor] = getKnobBodyGradientColors();

            juce::ColourGradient gradient(innerColor, centreX, centreY - radius * 0.3f,
                                           outerColor, centreX, centreY + radius * 0.5f,
                                           true);

            g.setGradientFill(gradient);
            g.fillEllipse(centreX - radius + 4.0f, centreY - radius + 4.0f,
                          (radius - 4.0f) * 2.0f, (radius - 4.0f) * 2.0f);

            // Border
            g.setColour(getKnobBorderColor());
            g.drawEllipse(centreX - radius + 4.0f, centreY - radius + 4.0f,
                          (radius - 4.0f) * 2.0f, (radius - 4.0f) * 2.0f, BORDER_WIDTH);
        }

        //==============================================================================
        // Center Cap
        //==============================================================================
        {
            float capRadius = radius * CENTER_CAP_PERCENTAGE;

            juce::ColourGradient capGradient(juce::Colour(0xff2a2a2a), centreX, centreY - capRadius * 0.5f,
                                              juce::Colour(0xff181818), centreX, centreY + capRadius * 0.5f,
                                              true);

            g.setGradientFill(capGradient);
            g.fillEllipse(centreX - capRadius, centreY - capRadius,
                          capRadius * 2.0f, capRadius * 2.0f);
        }

        //==============================================================================
        // Indicator Line
        //==============================================================================
        {
            float indicatorLength = radius * 0.5f;
            float indicatorStartRadius = radius * 0.2f;

            juce::Line<float> indicatorLine(
                centreX + indicatorStartRadius * std::cos(angle - juce::MathConstants<float>::halfPi),
                centreY + indicatorStartRadius * std::sin(angle - juce::MathConstants<float>::halfPi),
                centreX + indicatorLength * std::cos(angle - juce::MathConstants<float>::halfPi),
                centreY + indicatorLength * std::sin(angle - juce::MathConstants<float>::halfPi));

            g.setColour(accentColor);
            g.drawLine(indicatorLine, INDICATOR_WIDTH);
        }

        //==============================================================================
        // Arc Dot
        //==============================================================================
        {
            float dotRadius = 3.0f;
            float arcRadius = radius - 4.0f;
            float dotX = centreX + arcRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
            float dotY = centreY + arcRadius * std::sin(angle - juce::MathConstants<float>::halfPi);

            g.setColour(accentColor);
            g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
        }
    }

    //==============================================================================
    // Slider Layout
    //==============================================================================

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        auto localBounds = slider.getLocalBounds();

        juce::Slider::SliderLayout layout;

        if (slider.isRotary())
        {
            layout.sliderBounds = localBounds;
            layout.textBoxBounds = juce::Rectangle<int>();  // No built-in text box
        }
        else
        {
            layout = juce::LookAndFeel_V4::getSliderLayout(slider);
        }

        return layout;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReOrderLookAndFeel)
};
