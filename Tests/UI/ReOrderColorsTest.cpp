/*
  ==============================================================================

    ReOrderColorsTest.cpp
    Unit tests for ReOrderColors namespace

    Tests all color constants and helper functions as specified in
    TDD_IMPLEMENTATION_PLAN.md Iteration 1.

  ==============================================================================
*/

#include "../catch2/catch_amalgamated.hpp"
#include "../../Source/UI/ReOrderColors.h"

using namespace ReOrderColors;

//==============================================================================
// Background Color Tests
//==============================================================================

TEST_CASE("T1.1 Colors_BackgroundDarkestValue", "[colors][background]")
{
    REQUIRE(bgDarkest.getARGB() == 0xff0d0d0d);
}

TEST_CASE("T1.2 Colors_BackgroundDarkValue", "[colors][background]")
{
    REQUIRE(bgDark.getARGB() == 0xff1a1a1a);
}

TEST_CASE("T1.3 Colors_BackgroundModuleValue", "[colors][background]")
{
    REQUIRE(bgModule.getARGB() == 0xff141414);
}

TEST_CASE("T1.4 Colors_BorderValue", "[colors][border]")
{
    REQUIRE(border.getARGB() == 0xff2a2a2a);
}

TEST_CASE("T1.5 Colors_BorderLightValue", "[colors][border]")
{
    REQUIRE(borderLight.getARGB() == 0xff333333);
}

//==============================================================================
// Module Accent Color Tests
//==============================================================================

TEST_CASE("T1.6 Colors_AccentDelayValue", "[colors][accent]")
{
    REQUIRE(accentDelay.getARGB() == 0xffff66aa);
}

TEST_CASE("T1.7 Colors_AccentWidthValue", "[colors][accent]")
{
    REQUIRE(accentWidth.getARGB() == 0xff00d4ff);
}

TEST_CASE("T1.8 Colors_AccentPhaseValue", "[colors][accent]")
{
    REQUIRE(accentPhase.getARGB() == 0xffff9933);
}

TEST_CASE("T1.9 Colors_AccentOutputValue", "[colors][accent]")
{
    REQUIRE(accentOutput.getARGB() == 0xff00ff88);
}

//==============================================================================
// Text Color Tests
//==============================================================================

TEST_CASE("T1.10 Colors_TextPrimaryValue", "[colors][text]")
{
    REQUIRE(textPrimary.getARGB() == 0xffffffff);
}

TEST_CASE("T1.11 Colors_TextMutedValue", "[colors][text]")
{
    REQUIRE(textMuted.getARGB() == 0xff888888);
}

TEST_CASE("T1.12 Colors_TextDimValue", "[colors][text]")
{
    REQUIRE(textDim.getARGB() == 0xff666666);
}

//==============================================================================
// Status Color Tests
//==============================================================================

TEST_CASE("T1.13 Colors_StatusGoodValue", "[colors][status]")
{
    REQUIRE(statusGood.getARGB() == 0xff00ff88);
}

TEST_CASE("T1.14 Colors_StatusWarnValue", "[colors][status]")
{
    REQUIRE(statusWarn.getARGB() == 0xffffcc00);
}

TEST_CASE("T1.15 Colors_StatusErrorValue", "[colors][status]")
{
    REQUIRE(statusError.getARGB() == 0xffff3366);
}

//==============================================================================
// Helper Function Tests - getModuleAccent()
//==============================================================================

TEST_CASE("T1.16 Colors_GetModuleAccent_Delay", "[colors][helper]")
{
    REQUIRE(getModuleAccent(ModuleType::Delay) == accentDelay);
}

TEST_CASE("T1.17 Colors_GetModuleAccent_Width", "[colors][helper]")
{
    REQUIRE(getModuleAccent(ModuleType::Width) == accentWidth);
}

TEST_CASE("T1.18 Colors_GetModuleAccent_Phase", "[colors][helper]")
{
    REQUIRE(getModuleAccent(ModuleType::Phase) == accentPhase);
}

TEST_CASE("T1.19 Colors_GetModuleAccent_Output", "[colors][helper]")
{
    REQUIRE(getModuleAccent(ModuleType::Output) == accentOutput);
}

//==============================================================================
// Helper Function Tests - withAlpha()
//==============================================================================

TEST_CASE("T1.20 Colors_WithAlpha", "[colors][helper]")
{
    auto colorWithAlpha = withAlpha(accentDelay, 0.5f);

    // Check that RGB components are preserved
    REQUIRE(colorWithAlpha.getRed() == accentDelay.getRed());
    REQUIRE(colorWithAlpha.getGreen() == accentDelay.getGreen());
    REQUIRE(colorWithAlpha.getBlue() == accentDelay.getBlue());

    // Check that alpha is correctly set (0.5 * 255 ≈ 127-128)
    REQUIRE(colorWithAlpha.getAlpha() == Catch::Approx(127).margin(1));
}
