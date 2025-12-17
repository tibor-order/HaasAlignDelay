/*
  ==============================================================================

    PluginEditorLayoutTest.cpp
    Tests for Main Editor Layout (Iteration 21)

    Tests verify:
    - Default size (1000×625px)
    - Has header component
    - Has footer component
    - Has input meter strip
    - Has output meter strip
    - Has delay module
    - Has width module
    - Has phase module
    - Has output module
    - Modules in correct order
    - Meter strip positions
    - Uses ReOrderLookAndFeel
    - Timer for meters
    - Parameter connections

    Note: These tests use EditorLayout helper class for testability
    since full PluginEditor requires a running AudioProcessor.

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/EditorLayout.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T21.1 - Default Size (1000×625px)
//==============================================================================
TEST_CASE("T21.1 Editor_DefaultSize", "[ui][editor]")
{
    REQUIRE(EditorLayout::getDefaultWidth() == 1000);
    REQUIRE(EditorLayout::getDefaultHeight() == 625);
}

//==============================================================================
// T21.2 - Has Header
//==============================================================================
TEST_CASE("T21.2 Editor_HasHeader", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasHeader() == true);
}

//==============================================================================
// T21.3 - Has Footer
//==============================================================================
TEST_CASE("T21.3 Editor_HasFooter", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasFooter() == true);
}

//==============================================================================
// T21.4 - Has Input Meter
//==============================================================================
TEST_CASE("T21.4 Editor_HasInputMeter", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasInputMeter() == true);
}

//==============================================================================
// T21.5 - Has Output Meter
//==============================================================================
TEST_CASE("T21.5 Editor_HasOutputMeter", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasOutputMeter() == true);
}

//==============================================================================
// T21.6 - Has Delay Module
//==============================================================================
TEST_CASE("T21.6 Editor_HasDelayModule", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasDelayModule() == true);
}

//==============================================================================
// T21.7 - Has Width Module
//==============================================================================
TEST_CASE("T21.7 Editor_HasWidthModule", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasWidthModule() == true);
}

//==============================================================================
// T21.8 - Has Phase Module
//==============================================================================
TEST_CASE("T21.8 Editor_HasPhaseModule", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasPhaseModule() == true);
}

//==============================================================================
// T21.9 - Has Output Module
//==============================================================================
TEST_CASE("T21.9 Editor_HasOutputModule", "[ui][editor]")
{
    EditorLayout layout;
    REQUIRE(layout.hasOutputModule() == true);
}

//==============================================================================
// T21.10 - Modules In Order (L to R: Delay, Width, Phase, Output)
//==============================================================================
TEST_CASE("T21.10 Editor_ModulesInOrder", "[ui][editor]")
{
    EditorLayout layout;
    layout.setSize(EditorLayout::getDefaultWidth(), EditorLayout::getDefaultHeight());

    auto delayBounds = layout.getDelayModuleBounds();
    auto widthBounds = layout.getWidthModuleBounds();
    auto phaseBounds = layout.getPhaseModuleBounds();
    auto outputBounds = layout.getOutputModuleBounds();

    // Each module should be to the right of the previous
    REQUIRE(delayBounds.getRight() <= widthBounds.getX());
    REQUIRE(widthBounds.getRight() <= phaseBounds.getX());
    REQUIRE(phaseBounds.getRight() <= outputBounds.getX());
}

//==============================================================================
// T21.11 - Meter Strip Positions (input far left, output far right)
//==============================================================================
TEST_CASE("T21.11 Editor_MeterStripPositions", "[ui][editor]")
{
    EditorLayout layout;
    layout.setSize(EditorLayout::getDefaultWidth(), EditorLayout::getDefaultHeight());

    auto inputMeterBounds = layout.getInputMeterBounds();
    auto outputMeterBounds = layout.getOutputMeterBounds();

    // Input meter should be at far left
    REQUIRE(inputMeterBounds.getX() < 60);  // Allow some padding

    // Output meter should be at far right
    REQUIRE(outputMeterBounds.getRight() > EditorLayout::getDefaultWidth() - 60);
}

//==============================================================================
// T21.12 - Uses ReOrderLookAndFeel
//==============================================================================
TEST_CASE("T21.12 Editor_UsesReOrderLookAndFeel", "[ui][editor]")
{
    // The editor should be configured to use ReOrderLookAndFeel
    REQUIRE(EditorLayout::getLookAndFeelName() == "ReOrderLookAndFeel");
}

//==============================================================================
// T21.13 - Timer for Meters (refresh rate)
//==============================================================================
TEST_CASE("T21.13 Editor_TimerForMeters", "[ui][editor]")
{
    // Timer should refresh at ~30Hz for smooth meter animation
    int refreshRate = EditorLayout::getMeterRefreshRateHz();
    REQUIRE(refreshRate >= 25);
    REQUIRE(refreshRate <= 60);
}

//==============================================================================
// T21.14 - Parameter Connections (lists expected connections)
//==============================================================================
TEST_CASE("T21.14 Editor_ParameterConnections", "[ui][editor]")
{
    auto paramIds = EditorLayout::getRequiredParameterIds();

    // Should include all module parameters
    REQUIRE(paramIds.size() >= 10);

    // Check for key parameters
    bool hasDelayLeft = false;
    bool hasDelayRight = false;
    bool hasWidth = false;
    bool hasMix = false;

    for (const auto& id : paramIds)
    {
        if (id == "delayLeft") hasDelayLeft = true;
        if (id == "delayRight") hasDelayRight = true;
        if (id == "width") hasWidth = true;
        if (id == "mix") hasMix = true;
    }

    REQUIRE(hasDelayLeft);
    REQUIRE(hasDelayRight);
    REQUIRE(hasWidth);
    REQUIRE(hasMix);
}
