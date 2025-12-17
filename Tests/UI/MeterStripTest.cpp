/*
  ==============================================================================

    MeterStripTest.cpp
    Tests for Meter Strip Component (Iteration 20)

    Tests verify:
    - Width (50px)
    - Background color (#0d0d0d)
    - Border color (#2a2a2a)
    - Border radius (8px)
    - Has left meter
    - Has right meter
    - Has label
    - Has dB readout
    - Input label rotation (180°)
    - Output label rotation (0°)
    - dB format ("-12dB" or "-∞")
    - Updates from processor

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/MeterStrip.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T20.1 - Width (50px)
//==============================================================================
TEST_CASE("T20.1 MeterStrip_Width", "[ui][meterstrip]")
{
    float width = MeterStrip::getDefaultWidth();
    REQUIRE(width == Approx(50.0f));
}

//==============================================================================
// T20.2 - Background Color (#0d0d0d)
//==============================================================================
TEST_CASE("T20.2 MeterStrip_BackgroundColor", "[ui][meterstrip]")
{
    juce::Colour bgColor = MeterStrip::getBackgroundColor();
    REQUIRE(bgColor == ReOrderColors::bgDarkest);  // #0d0d0d
}

//==============================================================================
// T20.3 - Border Color (#2a2a2a)
//==============================================================================
TEST_CASE("T20.3 MeterStrip_BorderColor", "[ui][meterstrip]")
{
    juce::Colour borderColor = MeterStrip::getBorderColor();
    REQUIRE(borderColor == ReOrderColors::border);  // #2a2a2a
}

//==============================================================================
// T20.4 - Border Radius (8px)
//==============================================================================
TEST_CASE("T20.4 MeterStrip_BorderRadius", "[ui][meterstrip]")
{
    float radius = MeterStrip::getBorderRadius();
    REQUIRE(radius == Approx(8.0f));
}

//==============================================================================
// T20.5 - Has Left Meter
//==============================================================================
TEST_CASE("T20.5 MeterStrip_HasLeftMeter", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Input);
    REQUIRE(strip.getLeftMeter() != nullptr);
}

//==============================================================================
// T20.6 - Has Right Meter
//==============================================================================
TEST_CASE("T20.6 MeterStrip_HasRightMeter", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Input);
    REQUIRE(strip.getRightMeter() != nullptr);
}

//==============================================================================
// T20.7 - Has Label
//==============================================================================
TEST_CASE("T20.7 MeterStrip_HasLabel", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Input);
    REQUIRE(strip.hasLabel() == true);
}

//==============================================================================
// T20.8 - Has dB Readout
//==============================================================================
TEST_CASE("T20.8 MeterStrip_HasDbReadout", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Input);
    REQUIRE(strip.hasDbReadout() == true);
}

//==============================================================================
// T20.9 - Input Label Rotation (180°)
//==============================================================================
TEST_CASE("T20.9 MeterStrip_InputLabelRotation", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Input);
    float rotation = strip.getLabelRotationDegrees();
    REQUIRE(rotation == Approx(180.0f));
}

//==============================================================================
// T20.10 - Output Label Rotation (0°)
//==============================================================================
TEST_CASE("T20.10 MeterStrip_OutputLabelRotation", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Output);
    float rotation = strip.getLabelRotationDegrees();
    REQUIRE(rotation == Approx(0.0f));
}

//==============================================================================
// T20.11 - dB Format ("-12dB" or "-∞")
//==============================================================================
TEST_CASE("T20.11 MeterStrip_DbFormat", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Input);

    // Test with -12dB level
    juce::String dbText = strip.formatDbValue(-12.0f);
    REQUIRE(dbText.contains("-12"));
    REQUIRE(dbText.containsIgnoreCase("dB"));

    // Test with -infinity
    juce::String infText = strip.formatDbValue(-100.0f);  // Below threshold
    REQUIRE((infText.contains("-inf") || infText.contains("-∞") || infText.contains("inf")));
}

//==============================================================================
// T20.12 - Updates From Processor
//==============================================================================
TEST_CASE("T20.12 MeterStrip_UpdatesFromProcessor", "[ui][meterstrip]")
{
    MeterStrip strip(MeterStrip::Type::Output);

    // Set levels
    strip.setLevels(-6.0f, -3.0f);

    // Check that meters received the values
    REQUIRE(strip.getLeftLevel() == Approx(-6.0f));
    REQUIRE(strip.getRightLevel() == Approx(-3.0f));
}
