/*
  ==============================================================================

    DelayLinkTest.cpp
    Tests for Delay Link Feature (Iteration 6)

    Tests verify:
    - Link is disabled by default
    - Enabling/disabling link preserves current values
    - Adjusting one delay moves both when linked
    - Relative offset is maintained
    - Values clamp at boundaries (0-50ms)
    - Works with negative offset (R > L) and zero offset

  ==============================================================================
*/

#include "../catch2/catch_amalgamated.hpp"
#include "../../Source/DSP/HaasProcessor.h"
#include <cmath>

using Catch::Approx;

// Helper class to test delay link behavior
// In real implementation, this logic would be in PluginProcessor
class DelayLinkController
{
public:
    DelayLinkController() = default;

    void setLinked(bool linked) { isLinked = linked; }
    bool getLinked() const { return isLinked; }

    void setDelayLeft(float ms)
    {
        float delta = ms - delayLeftMs;
        delayLeftMs = clamp(ms);

        if (isLinked)
        {
            delayRightMs = clamp(delayRightMs + delta);
        }
    }

    void setDelayRight(float ms)
    {
        float delta = ms - delayRightMs;
        delayRightMs = clamp(ms);

        if (isLinked)
        {
            delayLeftMs = clamp(delayLeftMs + delta);
        }
    }

    float getDelayLeft() const { return delayLeftMs; }
    float getDelayRight() const { return delayRightMs; }

    // Direct setters for initialization (bypass link logic)
    void initDelayLeft(float ms) { delayLeftMs = clamp(ms); }
    void initDelayRight(float ms) { delayRightMs = clamp(ms); }

private:
    static float clamp(float ms)
    {
        return std::max(0.0f, std::min(50.0f, ms));
    }

    bool isLinked = false;
    float delayLeftMs = 0.0f;
    float delayRightMs = 0.0f;
};


TEST_CASE("T6.1 DelayLink_DefaultOff", "[dsp][delaylink]")
{
    DSP::HaasParameters params;
    REQUIRE(params.delayLink == false);
}

TEST_CASE("T6.2 DelayLink_EnableDoesNotChangeValues", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(10.0f);
    controller.initDelayRight(25.0f);

    // Enable link
    controller.setLinked(true);

    // Values should be unchanged
    REQUIRE(controller.getDelayLeft() == Approx(10.0f));
    REQUIRE(controller.getDelayRight() == Approx(25.0f));
}

TEST_CASE("T6.3 DelayLink_DisableDoesNotChangeValues", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(10.0f);
    controller.initDelayRight(25.0f);
    controller.setLinked(true);

    // Disable link
    controller.setLinked(false);

    // Values should be unchanged
    REQUIRE(controller.getDelayLeft() == Approx(10.0f));
    REQUIRE(controller.getDelayRight() == Approx(25.0f));
}

TEST_CASE("T6.4 DelayLink_AdjustLeft_MovesRight", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(10.0f);
    controller.initDelayRight(20.0f);
    controller.setLinked(true);

    // Increase left by 5ms
    controller.setDelayLeft(15.0f);

    // Right should also increase by 5ms
    REQUIRE(controller.getDelayLeft() == Approx(15.0f));
    REQUIRE(controller.getDelayRight() == Approx(25.0f));
}

TEST_CASE("T6.5 DelayLink_AdjustRight_MovesLeft", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(10.0f);
    controller.initDelayRight(20.0f);
    controller.setLinked(true);

    // Decrease right by 5ms
    controller.setDelayRight(15.0f);

    // Left should also decrease by 5ms
    REQUIRE(controller.getDelayLeft() == Approx(5.0f));
    REQUIRE(controller.getDelayRight() == Approx(15.0f));
}

TEST_CASE("T6.6 DelayLink_MaintainsOffset", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(5.0f);
    controller.initDelayRight(20.0f);  // 15ms offset
    controller.setLinked(true);

    // Increase left
    controller.setDelayLeft(10.0f);

    // Offset should be maintained (15ms)
    float offset = controller.getDelayRight() - controller.getDelayLeft();
    REQUIRE(offset == Approx(15.0f));
}

TEST_CASE("T6.7 DelayLink_ClampAtMax", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(30.0f);
    controller.initDelayRight(45.0f);  // 15ms offset
    controller.setLinked(true);

    // Try to increase left beyond what right can handle
    controller.setDelayLeft(40.0f);

    // Right should clamp at 50ms
    REQUIRE(controller.getDelayRight() == Approx(50.0f));
    // Left takes the full increase since it's applied first
    REQUIRE(controller.getDelayLeft() == Approx(40.0f));
}

TEST_CASE("T6.8 DelayLink_ClampAtMin", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(5.0f);
    controller.initDelayRight(20.0f);  // 15ms offset
    controller.setLinked(true);

    // Try to decrease right below what left can handle
    controller.setDelayRight(10.0f);

    // Left should clamp at 0ms
    REQUIRE(controller.getDelayLeft() == Approx(0.0f));
    // Right takes the full decrease since it's applied first
    REQUIRE(controller.getDelayRight() == Approx(10.0f));
}

TEST_CASE("T6.9 DelayLink_NegativeOffset", "[dsp][delaylink]")
{
    // R > L (negative offset from perspective of L - R)
    DelayLinkController controller;
    controller.initDelayLeft(25.0f);
    controller.initDelayRight(10.0f);  // L is ahead of R
    controller.setLinked(true);

    // Increase left
    controller.setDelayLeft(30.0f);

    // Offset should be maintained
    float offset = controller.getDelayLeft() - controller.getDelayRight();
    REQUIRE(offset == Approx(15.0f));
    REQUIRE(controller.getDelayRight() == Approx(15.0f));
}

TEST_CASE("T6.10 DelayLink_ZeroOffset", "[dsp][delaylink]")
{
    // L == R (zero offset)
    DelayLinkController controller;
    controller.initDelayLeft(20.0f);
    controller.initDelayRight(20.0f);
    controller.setLinked(true);

    // Adjust left
    controller.setDelayLeft(25.0f);

    // Both should move together
    REQUIRE(controller.getDelayLeft() == Approx(25.0f));
    REQUIRE(controller.getDelayRight() == Approx(25.0f));
}

TEST_CASE("T6.11 DelayLink_Unlinked_Independent", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(10.0f);
    controller.initDelayRight(20.0f);
    controller.setLinked(false);  // Explicitly unlinked

    // Adjust left
    controller.setDelayLeft(30.0f);

    // Right should be unchanged
    REQUIRE(controller.getDelayLeft() == Approx(30.0f));
    REQUIRE(controller.getDelayRight() == Approx(20.0f));
}

TEST_CASE("T6.12 DelayLink_RapidToggle", "[dsp][delaylink]")
{
    DelayLinkController controller;
    controller.initDelayLeft(10.0f);
    controller.initDelayRight(25.0f);

    // Rapid toggle shouldn't corrupt values
    for (int i = 0; i < 10; ++i)
    {
        controller.setLinked(true);
        controller.setLinked(false);
    }

    // Values should be preserved
    REQUIRE(controller.getDelayLeft() == Approx(10.0f));
    REQUIRE(controller.getDelayRight() == Approx(25.0f));
}
