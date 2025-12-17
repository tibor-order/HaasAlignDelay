/*
  ==============================================================================

    AnimationSystemTest.cpp
    Tests for Animation System (Iteration 24)

    Tests verify:
    - HaasIcon Y position oscillates over time
    - HaasIcon has 2 second period
    - WidthIcon scale changes over time
    - PhaseIcon animates when correcting
    - PhaseIcon does not animate when not correcting
    - OutputIcon opacity fades over time
    - Correcting badge opacity pulses
    - Correcting badge has 1 second period
    - Border transition duration is 300ms
    - Background tint fade duration is 200ms
    - Timer updates at 30Hz
    - Animation overhead is low CPU

  ==============================================================================
*/

#include "catch2/catch_amalgamated.hpp"
#include "../Source/UI/AnimationSystem.h"

using Catch::Approx;

//==============================================================================
// T24.1 - HaasIcon Y Position Oscillates
//==============================================================================
TEST_CASE("T24.1 Animation_HaasIcon_Oscillates", "[animation]")
{
    AnimationSystem::IconAnimator animator(AnimationSystem::IconType::Haas);

    // Get initial Y offset
    float initialY = animator.getYOffset();

    // Advance time
    animator.update(0.5f);  // 500ms
    float midY = animator.getYOffset();

    // Y should have changed
    REQUIRE(initialY != Approx(midY).margin(0.001f));
}

//==============================================================================
// T24.2 - HaasIcon Has 2 Second Period
//==============================================================================
TEST_CASE("T24.2 Animation_HaasIcon_Period", "[animation]")
{
    AnimationSystem::IconAnimator animator(AnimationSystem::IconType::Haas);

    // Get initial Y offset
    float initialY = animator.getYOffset();

    // Advance exactly one full period (2 seconds)
    animator.update(2.0f);
    float afterPeriodY = animator.getYOffset();

    // Should be back to approximately the same position
    REQUIRE(initialY == Approx(afterPeriodY).margin(0.01f));
}

//==============================================================================
// T24.3 - WidthIcon Scale Changes Over Time
//==============================================================================
TEST_CASE("T24.3 Animation_WidthIcon_Scales", "[animation]")
{
    AnimationSystem::IconAnimator animator(AnimationSystem::IconType::Width);

    // Get initial scale
    float initialScale = animator.getScale();

    // Advance time to half period (0.75s for 1.5s period)
    animator.update(0.375f);  // Quarter period
    float midScale = animator.getScale();

    // Scale should have changed
    REQUIRE(initialScale != Approx(midScale).margin(0.001f));
}

//==============================================================================
// T24.4 - PhaseIcon Animates When Correcting
//==============================================================================
TEST_CASE("T24.4 Animation_PhaseIcon_Active", "[animation]")
{
    AnimationSystem::IconAnimator animator(AnimationSystem::IconType::Phase);

    // Enable correcting state
    animator.setCorrecting(true);

    // Get initial compression
    float initialCompression = animator.getCompression();

    // Advance time
    animator.update(0.25f);
    float afterCompression = animator.getCompression();

    // Compression should change when correcting
    REQUIRE(initialCompression != Approx(afterCompression).margin(0.001f));
}

//==============================================================================
// T24.5 - PhaseIcon Does Not Animate When Not Correcting
//==============================================================================
TEST_CASE("T24.5 Animation_PhaseIcon_Inactive", "[animation]")
{
    AnimationSystem::IconAnimator animator(AnimationSystem::IconType::Phase);

    // Disable correcting state
    animator.setCorrecting(false);

    // Get initial compression
    float initialCompression = animator.getCompression();

    // Advance time
    animator.update(0.5f);
    float afterCompression = animator.getCompression();

    // Compression should remain at default (1.0) when not correcting
    REQUIRE(initialCompression == Approx(afterCompression).margin(0.001f));
    REQUIRE(afterCompression == Approx(1.0f).margin(0.001f));
}

//==============================================================================
// T24.6 - OutputIcon Opacity Fades Over Time
//==============================================================================
TEST_CASE("T24.6 Animation_OutputIcon_Fades", "[animation]")
{
    AnimationSystem::IconAnimator animator(AnimationSystem::IconType::Output);

    // Get initial opacity
    float initialOpacity = animator.getWaveOpacity();

    // Advance time
    animator.update(0.5f);
    float midOpacity = animator.getWaveOpacity();

    // Opacity should have changed
    REQUIRE(initialOpacity != Approx(midOpacity).margin(0.001f));
}

//==============================================================================
// T24.7 - Correcting Badge Opacity Pulses
//==============================================================================
TEST_CASE("T24.7 Animation_CorrectingBadge_Pulse", "[animation]")
{
    AnimationSystem::BadgeAnimator animator;

    // Get initial opacity
    float initialOpacity = animator.getOpacity();

    // Advance time to quarter period (0.25s for 1s period) - maximum difference
    animator.update(0.25f);
    float midOpacity = animator.getOpacity();

    // Opacity should have changed (at quarter period, sine reaches max)
    REQUIRE(initialOpacity != Approx(midOpacity).margin(0.01f));

    // Opacity should be between 0.5 and 1.0
    REQUIRE(midOpacity >= 0.5f);
    REQUIRE(midOpacity <= 1.0f);
}

//==============================================================================
// T24.8 - Correcting Badge Has 1 Second Period
//==============================================================================
TEST_CASE("T24.8 Animation_CorrectingBadge_Period", "[animation]")
{
    AnimationSystem::BadgeAnimator animator;

    // Get initial opacity
    float initialOpacity = animator.getOpacity();

    // Advance exactly one full period (1 second)
    animator.update(1.0f);
    float afterPeriodOpacity = animator.getOpacity();

    // Should be back to approximately the same opacity
    REQUIRE(initialOpacity == Approx(afterPeriodOpacity).margin(0.01f));
}

//==============================================================================
// T24.9 - Border Transition Duration is 300ms
//==============================================================================
TEST_CASE("T24.9 Animation_BorderTransition_Duration", "[animation]")
{
    AnimationSystem::TransitionAnimator animator(300.0f);  // 300ms duration

    // Start transition
    animator.startTransition(0.0f, 1.0f);

    // At 150ms (half duration), should be roughly halfway
    animator.update(150.0f);
    REQUIRE(animator.getCurrentValue() == Approx(0.5f).margin(0.1f));

    // At 300ms, should be at target
    animator.update(150.0f);
    REQUIRE(animator.getCurrentValue() == Approx(1.0f).margin(0.01f));

    // Verify transition duration
    REQUIRE(animator.getDurationMs() == Approx(300.0f));
}

//==============================================================================
// T24.10 - Background Tint Fade Duration is 200ms
//==============================================================================
TEST_CASE("T24.10 Animation_BackgroundTint_Duration", "[animation]")
{
    AnimationSystem::TransitionAnimator animator(200.0f);  // 200ms duration

    // Start transition
    animator.startTransition(0.0f, 1.0f);

    // At 100ms (half duration), should be roughly halfway
    animator.update(100.0f);
    REQUIRE(animator.getCurrentValue() == Approx(0.5f).margin(0.1f));

    // At 200ms, should be at target
    animator.update(100.0f);
    REQUIRE(animator.getCurrentValue() == Approx(1.0f).margin(0.01f));

    // Verify transition duration
    REQUIRE(animator.getDurationMs() == Approx(200.0f));
}

//==============================================================================
// T24.11 - Timer Updates at 30Hz
//==============================================================================
TEST_CASE("T24.11 Animation_Timer_30Hz", "[animation]")
{
    // 30Hz = ~33.33ms interval
    float expectedInterval = AnimationSystem::getTimerIntervalMs();

    REQUIRE(expectedInterval == Approx(33.33f).margin(1.0f));

    // Verify Hz calculation
    float hz = 1000.0f / expectedInterval;
    REQUIRE(hz == Approx(30.0f).margin(1.0f));
}

//==============================================================================
// T24.12 - Animation Overhead is Low CPU
//==============================================================================
TEST_CASE("T24.12 Animation_LowCPU", "[animation]")
{
    // Create all animator types
    AnimationSystem::IconAnimator haasAnim(AnimationSystem::IconType::Haas);
    AnimationSystem::IconAnimator widthAnim(AnimationSystem::IconType::Width);
    AnimationSystem::IconAnimator phaseAnim(AnimationSystem::IconType::Phase);
    AnimationSystem::IconAnimator outputAnim(AnimationSystem::IconType::Output);
    AnimationSystem::BadgeAnimator badgeAnim;
    AnimationSystem::TransitionAnimator borderAnim(300.0f);
    AnimationSystem::TransitionAnimator bgAnim(200.0f);

    // Measure time for 1000 update cycles
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i)
    {
        float dt = 0.033f;  // ~30Hz
        haasAnim.update(dt);
        widthAnim.update(dt);
        phaseAnim.update(dt);
        outputAnim.update(dt);
        badgeAnim.update(dt);
        borderAnim.update(dt * 1000.0f);  // ms
        bgAnim.update(dt * 1000.0f);       // ms

        // Get all values (simulate paint)
        (void)haasAnim.getYOffset();
        (void)widthAnim.getScale();
        (void)phaseAnim.getCompression();
        (void)outputAnim.getWaveOpacity();
        (void)badgeAnim.getOpacity();
        (void)borderAnim.getCurrentValue();
        (void)bgAnim.getCurrentValue();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // 1000 iterations of all animations should complete in < 10ms
    // This represents ~33 seconds of animation time compressed into real execution
    // At 30Hz, that's about 1000 frames worth of animation
    REQUIRE(duration < 10000);  // Less than 10ms for 1000 updates

    // Calculate approximate CPU overhead per frame at 30Hz
    // Real-time frame budget is 33.33ms, our update should be < 0.01ms
    float msPerFrame = static_cast<float>(duration) / 1000.0f / 1000.0f;  // Convert to ms
    float cpuPercent = (msPerFrame / 33.33f) * 100.0f;

    // Should be less than 1% CPU
    REQUIRE(cpuPercent < 1.0f);
}
