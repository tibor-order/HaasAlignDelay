/*
  ==============================================================================

    ReOrderColors.h
    ReOrder Audio Design System - Color Palette

    This file defines the complete color system for ReOrder Audio plugins,
    including background colors, module accent colors, text colors, and
    status indicators as specified in the design system.

  ==============================================================================
*/

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace ReOrderColors
{

//==============================================================================
// Background Colors
//==============================================================================

/** Darkest background color - used for footer/header */
const juce::Colour bgDarkest    { 0xff0d0d0d };

/** Primary background color - used for main plugin body */
const juce::Colour bgDark       { 0xff1a1a1a };

/** Module panel background color - slightly lighter for module panels */
const juce::Colour bgModule     { 0xff141414 };

/** Primary border color - subtle separation between elements */
const juce::Colour border       { 0xff2a2a2a };

/** Secondary border color - used for input fields, buttons */
const juce::Colour borderLight  { 0xff333333 };

//==============================================================================
// Module Accent Colors
//==============================================================================

/** Delay module accent - Pink/Magenta */
const juce::Colour accentDelay  { 0xffff66aa };

/** Width/Stereo module accent - Cyan */
const juce::Colour accentWidth  { 0xff00d4ff };

/** Phase/Correction module accent - Orange */
const juce::Colour accentPhase  { 0xffff9933 };

/** Output/Master module accent - Green */
const juce::Colour accentOutput { 0xff00ff88 };

/** EQ/Tone module accent - Purple */
const juce::Colour accentEQ     { 0xffaa66ff };

/** Saturation module accent - Gold/Yellow */
const juce::Colour accentSat    { 0xffffcc00 };

/** Reverb/Space module accent - Blue */
const juce::Colour accentReverb { 0xff6688ff };

/** Compression module accent - Red */
const juce::Colour accentComp   { 0xffff6666 };

//==============================================================================
// Text Colors
//==============================================================================

/** Primary text color - white */
const juce::Colour textPrimary  { 0xffffffff };

/** Secondary text color - light gray */
const juce::Colour textSecondary { 0xffcccccc };

/** Muted text color - medium gray */
const juce::Colour textMuted    { 0xff888888 };

/** Dim text color - dark gray */
const juce::Colour textDim      { 0xff666666 };

/** Very muted text color - very dark gray */
const juce::Colour textVeryMuted { 0xff444444 };

//==============================================================================
// Status Colors
//==============================================================================

/** Good/Safe status - green */
const juce::Colour statusGood   { 0xff00ff88 };

/** Warning status - yellow/gold */
const juce::Colour statusWarn   { 0xffffcc00 };

/** Error/Clip status - red */
const juce::Colour statusError  { 0xffff3366 };

/** Inactive/Off state - dark gray */
const juce::Colour statusOff    { 0xff444444 };

//==============================================================================
// Module Type Enumeration
//==============================================================================

/** Enum defining the types of modules in the plugin */
enum class ModuleType
{
    Delay,
    Width,
    Phase,
    Output,
    EQ,
    Saturation,
    Reverb,
    Compression
};

//==============================================================================
// Helper Functions
//==============================================================================

/**
 * Returns the accent color for a given module type.
 *
 * @param moduleType The type of module
 * @return The juce::Colour accent color for that module
 */
inline juce::Colour getModuleAccent(ModuleType moduleType)
{
    switch (moduleType)
    {
        case ModuleType::Delay:       return accentDelay;
        case ModuleType::Width:       return accentWidth;
        case ModuleType::Phase:       return accentPhase;
        case ModuleType::Output:      return accentOutput;
        case ModuleType::EQ:          return accentEQ;
        case ModuleType::Saturation:  return accentSat;
        case ModuleType::Reverb:      return accentReverb;
        case ModuleType::Compression: return accentComp;
        default:                      return accentDelay;
    }
}

/**
 * Returns a color with modified alpha value.
 *
 * @param colour The base color
 * @param alpha The alpha value (0.0f to 1.0f)
 * @return The color with the new alpha value
 */
inline juce::Colour withAlpha(const juce::Colour& colour, float alpha)
{
    return colour.withAlpha(alpha);
}

} // namespace ReOrderColors
