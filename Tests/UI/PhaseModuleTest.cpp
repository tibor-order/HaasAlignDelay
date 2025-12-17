/*
  ==============================================================================

    PhaseModuleTest.cpp
    Tests for Phase Module Component (Iteration 17)

    Tests verify:
    - Accent color (orange #ff9933)
    - Correlation meter exists
    - AUTO FIX toggle exists
    - Threshold knob exists
    - Threshold range (0-1.0)
    - Speed knob exists
    - Speed range (0-100%)
    - Phase icon exists
    - EXCLUSIVE badge exists
    - Correcting badge hidden when not correcting
    - Correcting badge visible when correcting
    - Correcting badge pulses
    - Active glow when correcting
    - Parameter attachment

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/PhaseModule.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T17.1 - Accent Color (orange #ff9933)
//==============================================================================
TEST_CASE("T17.1 PhaseModule_AccentColor", "[ui][phasemodule]")
{
    PhaseModule module;
    REQUIRE(module.getAccentColor() == ReOrderColors::accentPhase);  // #ff9933
}

//==============================================================================
// T17.2 - Has Correlation Meter
//==============================================================================
TEST_CASE("T17.2 PhaseModule_HasCorrelationMeter", "[ui][phasemodule]")
{
    PhaseModule module;
    auto* meter = module.getCorrelationMeter();
    REQUIRE(meter != nullptr);
}

//==============================================================================
// T17.3 - Has AUTO FIX Toggle
//==============================================================================
TEST_CASE("T17.3 PhaseModule_HasAutoFixToggle", "[ui][phasemodule]")
{
    PhaseModule module;
    auto* autoFixButton = module.getAutoFixButton();
    REQUIRE(autoFixButton != nullptr);
}

//==============================================================================
// T17.4 - Has Threshold Knob
//==============================================================================
TEST_CASE("T17.4 PhaseModule_HasThresholdKnob", "[ui][phasemodule]")
{
    PhaseModule module;
    auto* thresholdKnob = module.getThresholdKnob();
    REQUIRE(thresholdKnob != nullptr);
}

//==============================================================================
// T17.5 - Threshold Range (0-1.0)
//==============================================================================
TEST_CASE("T17.5 PhaseModule_ThresholdRange", "[ui][phasemodule]")
{
    PhaseModule module;
    auto* thresholdKnob = module.getThresholdKnob();

    REQUIRE(thresholdKnob->getMinimum() == Approx(0.0));
    REQUIRE(thresholdKnob->getMaximum() == Approx(1.0));
}

//==============================================================================
// T17.6 - Has Speed Knob
//==============================================================================
TEST_CASE("T17.6 PhaseModule_HasSpeedKnob", "[ui][phasemodule]")
{
    PhaseModule module;
    auto* speedKnob = module.getSpeedKnob();
    REQUIRE(speedKnob != nullptr);
}

//==============================================================================
// T17.7 - Speed Range (0-100%)
//==============================================================================
TEST_CASE("T17.7 PhaseModule_SpeedRange", "[ui][phasemodule]")
{
    PhaseModule module;
    auto* speedKnob = module.getSpeedKnob();

    REQUIRE(speedKnob->getMinimum() == Approx(0.0));
    REQUIRE(speedKnob->getMaximum() == Approx(100.0));
}

//==============================================================================
// T17.8 - Has Icon
//==============================================================================
TEST_CASE("T17.8 PhaseModule_HasIcon", "[ui][phasemodule]")
{
    PhaseModule module;
    REQUIRE(module.hasIcon() == true);
}

//==============================================================================
// T17.9 - Has EXCLUSIVE Badge
//==============================================================================
TEST_CASE("T17.9 PhaseModule_HasExclusiveBadge", "[ui][phasemodule]")
{
    PhaseModule module;
    REQUIRE(module.hasExclusiveBadge() == true);
}

//==============================================================================
// T17.10 - Correcting Badge Hidden When Not Correcting
//==============================================================================
TEST_CASE("T17.10 PhaseModule_CorrectingBadge_Hidden", "[ui][phasemodule]")
{
    PhaseModule module;

    module.setCorrecting(false);
    REQUIRE(module.isCorrectingBadgeVisible() == false);
}

//==============================================================================
// T17.11 - Correcting Badge Visible When Correcting
//==============================================================================
TEST_CASE("T17.11 PhaseModule_CorrectingBadge_Visible", "[ui][phasemodule]")
{
    PhaseModule module;

    module.setCorrecting(true);
    REQUIRE(module.isCorrectingBadgeVisible() == true);
}

//==============================================================================
// T17.12 - Correcting Badge Pulses
//==============================================================================
TEST_CASE("T17.12 PhaseModule_CorrectingBadge_Pulses", "[ui][phasemodule]")
{
    PhaseModule module;

    module.setCorrecting(true);

    // Get pulse opacity at different times
    float opacity1 = module.getCorrectingBadgeOpacity(0.0f);    // sin(0) = 0 -> 0.5
    float opacity2 = module.getCorrectingBadgeOpacity(0.25f);   // sin(π/2) = 1 -> 1.0

    // Opacity should vary (pulse effect)
    REQUIRE(opacity1 != Approx(opacity2).margin(0.1f));

    // Check range is correct
    REQUIRE(opacity1 >= 0.5f);
    REQUIRE(opacity1 <= 1.0f);
    REQUIRE(opacity2 >= 0.5f);
    REQUIRE(opacity2 <= 1.0f);
}

//==============================================================================
// T17.13 - Active Glow When Correcting
//==============================================================================
TEST_CASE("T17.13 PhaseModule_ActiveGlow", "[ui][phasemodule]")
{
    PhaseModule module;

    module.setCorrecting(false);
    REQUIRE(module.hasActiveGlow() == false);

    module.setCorrecting(true);
    REQUIRE(module.hasActiveGlow() == true);
}

//==============================================================================
// T17.14 - Parameter Attachment Ready
//==============================================================================
TEST_CASE("T17.14 PhaseModule_ParameterAttachment", "[ui][phasemodule]")
{
    PhaseModule module;

    // Verify controls are ready for attachment
    auto* thresholdKnob = module.getThresholdKnob();
    auto* speedKnob = module.getSpeedKnob();

    REQUIRE(thresholdKnob->isEnabled());
    REQUIRE(speedKnob->isEnabled());

    // Verify the module exposes parameter IDs
    REQUIRE(PhaseModule::getThresholdParamID() == "phaseThreshold");
    REQUIRE(PhaseModule::getSpeedParamID() == "correctionSpeed");
    REQUIRE(PhaseModule::getAutoFixParamID() == "autoPhaseCorrection");
    REQUIRE(PhaseModule::getBypassParamID() == "phaseBypass");
}
