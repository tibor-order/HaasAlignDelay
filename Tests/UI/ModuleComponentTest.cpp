/*
  ==============================================================================

    ModuleComponentTest.cpp
    Tests for Module Component Base Class (Iteration 14)

    Tests verify:
    - Background color (#141414)
    - Border color (#2a2a2a)
    - Border radius (10px)
    - Header height (40px)
    - Power button position in header
    - Title uppercase formatting
    - Title uses accent color
    - Inactive opacity (30-40%)
    - Active glow effect
    - Accent color property
    - Icon area exists
    - Controls area exists

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/ModuleComponent.h"
#include "../Source/UI/ReOrderColors.h"

using Catch::Approx;

//==============================================================================
// T14.1 - Background Color (#141414)
//==============================================================================
TEST_CASE("T14.1 ModuleComponent_BackgroundColor", "[ui][modulecomponent]")
{
    juce::Colour bgColor = ModuleComponent::getBackgroundColor();
    REQUIRE(bgColor == ReOrderColors::bgModule);  // #141414
}

//==============================================================================
// T14.2 - Border Color (#2a2a2a)
//==============================================================================
TEST_CASE("T14.2 ModuleComponent_BorderColor", "[ui][modulecomponent]")
{
    juce::Colour borderColor = ModuleComponent::getBorderColor();
    REQUIRE(borderColor == ReOrderColors::border);  // #2a2a2a
}

//==============================================================================
// T14.3 - Border Radius (10px)
//==============================================================================
TEST_CASE("T14.3 ModuleComponent_BorderRadius", "[ui][modulecomponent]")
{
    float radius = ModuleComponent::getBorderRadius();
    REQUIRE(radius == Approx(10.0f));
}

//==============================================================================
// T14.4 - Header Height (40px)
//==============================================================================
TEST_CASE("T14.4 ModuleComponent_HeaderHeight", "[ui][modulecomponent]")
{
    float headerHeight = ModuleComponent::getHeaderHeight();
    REQUIRE(headerHeight == Approx(40.0f));
}

//==============================================================================
// T14.5 - Power Button In Header
//==============================================================================
TEST_CASE("T14.5 ModuleComponent_PowerButtonInHeader", "[ui][modulecomponent]")
{
    ModuleComponent module("Test", ReOrderColors::accentDelay);

    // Power button should exist and be in header area
    auto* powerButton = module.getPowerButton();
    REQUIRE(powerButton != nullptr);

    // Power button should be at left of header
    auto headerBounds = module.getHeaderBounds();
    auto buttonBounds = powerButton->getBounds();

    // Button should be within header vertically
    REQUIRE(buttonBounds.getY() >= headerBounds.getY());
    REQUIRE(buttonBounds.getBottom() <= headerBounds.getBottom());
}

//==============================================================================
// T14.6 - Title Uppercase
//==============================================================================
TEST_CASE("T14.6 ModuleComponent_TitleUppercase", "[ui][modulecomponent]")
{
    ModuleComponent module("delay", ReOrderColors::accentDelay);

    juce::String displayTitle = module.getDisplayTitle();
    REQUIRE(displayTitle == "DELAY");
}

//==============================================================================
// T14.7 - Title Color (accent color)
//==============================================================================
TEST_CASE("T14.7 ModuleComponent_TitleColor", "[ui][modulecomponent]")
{
    ModuleComponent module("Test", ReOrderColors::accentWidth);

    juce::Colour titleColor = module.getTitleColor();
    REQUIRE(titleColor == ReOrderColors::accentWidth);
}

//==============================================================================
// T14.8 - Inactive Opacity (30-40%)
//==============================================================================
TEST_CASE("T14.8 ModuleComponent_InactiveOpacity", "[ui][modulecomponent]")
{
    float inactiveOpacity = ModuleComponent::getInactiveOpacity();
    REQUIRE(inactiveOpacity >= 0.30f);
    REQUIRE(inactiveOpacity <= 0.40f);
}

//==============================================================================
// T14.9 - Active Glow
//==============================================================================
TEST_CASE("T14.9 ModuleComponent_ActiveGlow", "[ui][modulecomponent]")
{
    ModuleComponent module("Test", ReOrderColors::accentPhase);

    module.setActive(true);
    REQUIRE(module.hasActiveGlow() == true);

    module.setActive(false);
    REQUIRE(module.hasActiveGlow() == false);
}

//==============================================================================
// T14.10 - Accent Color Property
//==============================================================================
TEST_CASE("T14.10 ModuleComponent_AccentColorProperty", "[ui][modulecomponent]")
{
    ModuleComponent module("Test", ReOrderColors::accentDelay);

    REQUIRE(module.getAccentColor() == ReOrderColors::accentDelay);

    module.setAccentColor(ReOrderColors::accentOutput);
    REQUIRE(module.getAccentColor() == ReOrderColors::accentOutput);
}

//==============================================================================
// T14.11 - Icon Area Exists
//==============================================================================
TEST_CASE("T14.11 ModuleComponent_IconAreaExists", "[ui][modulecomponent]")
{
    ModuleComponent module("Test", ReOrderColors::accentDelay);
    module.setSize(200, 400);

    auto iconBounds = module.getIconBounds();

    // Icon area should have meaningful dimensions
    REQUIRE(iconBounds.getWidth() > 0);
    REQUIRE(iconBounds.getHeight() >= 60);  // At least 60px for icon
}

//==============================================================================
// T14.12 - Controls Area Exists
//==============================================================================
TEST_CASE("T14.12 ModuleComponent_ControlsAreaExists", "[ui][modulecomponent]")
{
    ModuleComponent module("Test", ReOrderColors::accentDelay);
    module.setSize(200, 400);

    auto controlsBounds = module.getControlsBounds();

    // Controls area should have meaningful dimensions
    REQUIRE(controlsBounds.getWidth() > 0);
    REQUIRE(controlsBounds.getHeight() > 0);

    // Controls area should be below header
    REQUIRE(controlsBounds.getY() >= ModuleComponent::getHeaderHeight());
}
