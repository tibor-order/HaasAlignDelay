/*
  ==============================================================================

    ModuleIconsTest.cpp
    Tests for Module Icons (Iteration 10)

    Tests verify:
    - Icon paths exist and are valid
    - Icons fit within 60×60px bounds
    - Stroke width defaults
    - Correct colors for each module type
    - Glow effects for active/inactive states

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/ModuleIcons.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T10.1 - HaasIcon Path Exists
//==============================================================================
TEST_CASE("T10.1 ModuleIcons_HaasIcon_PathExists", "[ui][icons]")
{
    juce::Path path = ModuleIcons::getHaasIconPath();
    REQUIRE_FALSE(path.isEmpty());
}

//==============================================================================
// T10.2 - WidthIcon Path Exists
//==============================================================================
TEST_CASE("T10.2 ModuleIcons_WidthIcon_PathExists", "[ui][icons]")
{
    juce::Path path = ModuleIcons::getWidthIconPath();
    REQUIRE_FALSE(path.isEmpty());
}

//==============================================================================
// T10.3 - PhaseIcon Path Exists
//==============================================================================
TEST_CASE("T10.3 ModuleIcons_PhaseIcon_PathExists", "[ui][icons]")
{
    juce::Path path = ModuleIcons::getPhaseIconPath();
    REQUIRE_FALSE(path.isEmpty());
}

//==============================================================================
// T10.4 - OutputIcon Path Exists
//==============================================================================
TEST_CASE("T10.4 ModuleIcons_OutputIcon_PathExists", "[ui][icons]")
{
    juce::Path path = ModuleIcons::getOutputIconPath();
    REQUIRE_FALSE(path.isEmpty());
}

//==============================================================================
// T10.5 - All Icons Fit Within 60×60 Bounds
//==============================================================================
TEST_CASE("T10.5 ModuleIcons_BoundsCheck", "[ui][icons]")
{
    constexpr float MAX_SIZE = 60.0f;

    // HaasIcon
    {
        juce::Path path = ModuleIcons::getHaasIconPath();
        auto bounds = path.getBounds();
        REQUIRE(bounds.getWidth() <= MAX_SIZE);
        REQUIRE(bounds.getHeight() <= MAX_SIZE);
    }

    // WidthIcon
    {
        juce::Path path = ModuleIcons::getWidthIconPath();
        auto bounds = path.getBounds();
        REQUIRE(bounds.getWidth() <= MAX_SIZE);
        REQUIRE(bounds.getHeight() <= MAX_SIZE);
    }

    // PhaseIcon
    {
        juce::Path path = ModuleIcons::getPhaseIconPath();
        auto bounds = path.getBounds();
        REQUIRE(bounds.getWidth() <= MAX_SIZE);
        REQUIRE(bounds.getHeight() <= MAX_SIZE);
    }

    // OutputIcon
    {
        juce::Path path = ModuleIcons::getOutputIconPath();
        auto bounds = path.getBounds();
        REQUIRE(bounds.getWidth() <= MAX_SIZE);
        REQUIRE(bounds.getHeight() <= MAX_SIZE);
    }
}

//==============================================================================
// T10.6 - Default Stroke Width
//==============================================================================
TEST_CASE("T10.6 ModuleIcons_StrokeWidth", "[ui][icons]")
{
    REQUIRE(ModuleIcons::getDefaultStrokeWidth() == Approx(2.0f));
}

//==============================================================================
// T10.7 - HaasIcon Color (pink)
//==============================================================================
TEST_CASE("T10.7 ModuleIcons_HaasIcon_Color", "[ui][icons]")
{
    juce::Colour color = ModuleIcons::getIconColor(ModuleIcons::IconType::Haas);
    REQUIRE(color == ReOrderColors::accentDelay);  // Pink: #ff66aa
}

//==============================================================================
// T10.8 - WidthIcon Color (cyan)
//==============================================================================
TEST_CASE("T10.8 ModuleIcons_WidthIcon_Color", "[ui][icons]")
{
    juce::Colour color = ModuleIcons::getIconColor(ModuleIcons::IconType::Width);
    REQUIRE(color == ReOrderColors::accentWidth);  // Cyan: #00d4ff
}

//==============================================================================
// T10.9 - PhaseIcon Color (orange)
//==============================================================================
TEST_CASE("T10.9 ModuleIcons_PhaseIcon_Color", "[ui][icons]")
{
    juce::Colour color = ModuleIcons::getIconColor(ModuleIcons::IconType::Phase);
    REQUIRE(color == ReOrderColors::accentPhase);  // Orange: #ff9933
}

//==============================================================================
// T10.10 - OutputIcon Color (green)
//==============================================================================
TEST_CASE("T10.10 ModuleIcons_OutputIcon_Color", "[ui][icons]")
{
    juce::Colour color = ModuleIcons::getIconColor(ModuleIcons::IconType::Output);
    REQUIRE(color == ReOrderColors::accentOutput);  // Green: #00ff88
}

//==============================================================================
// T10.11 - Active State Adds Glow
//==============================================================================
TEST_CASE("T10.11 ModuleIcons_ActiveGlow", "[ui][icons]")
{
    // Active state should have non-zero glow amount
    float glowAmount = ModuleIcons::getGlowAmount(true);
    REQUIRE(glowAmount > 0.0f);
}

//==============================================================================
// T10.12 - Inactive State No Glow
//==============================================================================
TEST_CASE("T10.12 ModuleIcons_InactiveNoGlow", "[ui][icons]")
{
    // Inactive state should have zero or minimal glow
    float glowAmount = ModuleIcons::getGlowAmount(false);
    REQUIRE(glowAmount == Approx(0.0f));
}
