#pragma once

#include <JuceHeader.h>

namespace UI
{

/**
 * @brief NuroColors - Waves X-Vox Pro / Nuro Audio Color Palette
 *
 * Complete color system based on the Waves X-Vox Pro reference image.
 * Primary accent: Cyan (#00d9ff) - the signature Waves/Nuro look.
 */
namespace NuroColors
{
    // === BACKGROUNDS ===
    constexpr uint32 backgroundBlack        = 0xff0a0a0a;  // Main background - deep black
    constexpr uint32 backgroundDark         = 0xff0f0f0f;  // Secondary background
    constexpr uint32 panelBackground        = 0xff1a1a1a;  // Panel sections - dark charcoal
    constexpr uint32 panelBackgroundLight   = 0xff1e1e1e;  // Slightly lighter panels
    constexpr uint32 moduleBackground       = 0xff252525;  // Module backgrounds
    constexpr uint32 moduleBackgroundLight  = 0xff2a2a2a;  // Lighter module areas
    constexpr uint32 borderDark             = 0xff0d0d0d;  // Subtle dark borders
    constexpr uint32 borderMedium           = 0xff333333;  // Medium border color

    // === PRIMARY ACCENT: CYAN (Signature Waves/Nuro look) ===
    constexpr uint32 accentCyan             = 0xff00d9ff;  // Primary accent
    constexpr uint32 accentCyanBright       = 0xff4dd9ff;  // Brighter cyan for highlights
    constexpr uint32 accentCyanDark         = 0xff00a8cc;  // Darker cyan for depth
    constexpr uint32 glowCyan               = 0x8000d9ff;  // Cyan with 50% alpha for glow
    constexpr uint32 glowCyanSoft           = 0x4000d9ff;  // Softer glow (25% alpha)
    constexpr uint32 glowCyanStrong         = 0xB000d9ff;  // Stronger glow (69% alpha)

    // === SECONDARY ACCENTS ===
    constexpr uint32 accentPink             = 0xffff3d8f;  // Pink/Magenta (like DYNAMICS)
    constexpr uint32 accentPinkBright       = 0xffff6ba8;
    constexpr uint32 accentPurple           = 0xffa855f7;  // Purple (like SPACE)
    constexpr uint32 accentPurpleBright     = 0xffc084fc;
    constexpr uint32 accentYellow           = 0xfffbbf24;  // Yellow/Gold (like SFX)
    constexpr uint32 accentYellowBright     = 0xfffcd34d;
    constexpr uint32 accentOrange           = 0xffff9500;  // Orange for warnings

    // === TEXT COLORS ===
    constexpr uint32 textWhite              = 0xffffffff;  // Primary text - pure white
    constexpr uint32 textLight              = 0xffd0d0d0;  // Light grey text
    constexpr uint32 textSecondary          = 0xffb0b0b0;  // Secondary text - light grey
    constexpr uint32 textDisabled           = 0xff606060;  // Disabled text - dark grey
    constexpr uint32 textMuted              = 0xff808080;  // Muted text for labels

    // === METER COLORS ===
    constexpr uint32 meterGreen             = 0xff00ff88;  // Meter green zone
    constexpr uint32 meterGreenBright       = 0xff00ffaa;
    constexpr uint32 meterYellow            = 0xfffbbf24;  // Meter yellow zone
    constexpr uint32 meterOrange            = 0xffff9500;
    constexpr uint32 meterRed               = 0xffff4444;  // Meter red zone
    constexpr uint32 meterRedBright         = 0xffff0000;
    constexpr uint32 meterPeakCyan          = 0xff00ffff;  // Peak indicators

    // === KNOB COLORS ===
    constexpr uint32 knobBase               = 0xff2a2a2a;  // Knob base dark grey
    constexpr uint32 knobBaseLight          = 0xff333333;  // Slightly lighter
    constexpr uint32 knobInner              = 0xff1e1e1e;  // Inner ring darker
    constexpr uint32 knobSurface            = 0xff353535;  // Knob surface center
    constexpr uint32 knobSurfaceEdge        = 0xff282828;  // Knob surface edge
    constexpr uint32 knobIndicatorWhite     = 0xffffffff;  // White indicator dot

    // === HELPER: Get JUCE Colour from uint32 ===
    inline juce::Colour colour(uint32 c) { return juce::Colour(c); }

    // === GLOW PRESETS ===
    namespace Glow
    {
        // Subtle glow: 0 0 8px accent, 0 0 15px accent40
        inline juce::Colour subtle(uint32 accent)     { return juce::Colour(accent).withAlpha(0.5f); }
        inline juce::Colour medium(uint32 accent)     { return juce::Colour(accent).withAlpha(0.6f); }
        inline juce::Colour strong(uint32 accent)     { return juce::Colour(accent).withAlpha(0.8f); }
        inline juce::Colour bloom(uint32 accent)      { return juce::Colour(accent).withAlpha(0.25f); }
    }

    // === SIZING CONSTANTS ===
    namespace Sizes
    {
        constexpr int knobSizeLarge         = 85;   // Main controls
        constexpr int knobSizeSmall         = 65;   // Secondary controls
        constexpr int knobArcWidth          = 4;    // Arc indicator width
        constexpr int knobGlowRadius        = 15;   // Glow blur radius
        constexpr int panelPadding          = 25;   // Outer panel padding
        constexpr int controlSpacing        = 20;   // Between controls
        constexpr int borderRadius          = 8;    // Panel corner radius
        constexpr float glowOpacity         = 0.6f; // Default glow opacity
    }

} // namespace NuroColors

} // namespace UI
