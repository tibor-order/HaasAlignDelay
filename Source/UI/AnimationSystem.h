/*
  ==============================================================================

    AnimationSystem.h
    Animation utilities for module icons and UI state transitions

    Features:
    - Icon animations (HaasIcon, WidthIcon, PhaseIcon, OutputIcon)
    - Badge pulse animation for "CORRECTING" indicator
    - Smooth transitions for border color and background tint
    - 30Hz timer-driven updates
    - Low CPU overhead

    Animation Details:
    - HaasIcon: Wave motion (translateY oscillation, 2s period)
    - WidthIcon: Expand/contract arrows (scale, 1.5s period)
    - PhaseIcon: Wave compression when correcting
    - OutputIcon: Sound wave fade animation
    - Badge: Pulse opacity 1.0 ↔ 0.5 (1s period)
    - Border transition: 300ms
    - Background tint: 200ms

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>
#include <chrono>

namespace AnimationSystem
{

//==============================================================================
// Constants
//==============================================================================

constexpr float TIMER_HZ = 30.0f;
constexpr float TIMER_INTERVAL_MS = 1000.0f / TIMER_HZ;

constexpr float HAAS_ICON_PERIOD = 2.0f;          // seconds
constexpr float HAAS_ICON_AMPLITUDE = 3.0f;       // pixels

constexpr float WIDTH_ICON_PERIOD = 1.5f;         // seconds
constexpr float WIDTH_ICON_MIN_SCALE = 0.9f;
constexpr float WIDTH_ICON_MAX_SCALE = 1.1f;

constexpr float PHASE_ICON_PERIOD = 0.8f;         // seconds
constexpr float PHASE_ICON_MIN_COMPRESSION = 0.8f;
constexpr float PHASE_ICON_MAX_COMPRESSION = 1.0f;

constexpr float OUTPUT_ICON_PERIOD = 1.2f;        // seconds
constexpr float OUTPUT_ICON_MIN_OPACITY = 0.3f;
constexpr float OUTPUT_ICON_MAX_OPACITY = 1.0f;

constexpr float BADGE_PERIOD = 1.0f;              // seconds
constexpr float BADGE_MIN_OPACITY = 0.5f;
constexpr float BADGE_MAX_OPACITY = 1.0f;

constexpr float BORDER_TRANSITION_MS = 300.0f;
constexpr float BACKGROUND_TINT_MS = 200.0f;

//==============================================================================
// Utility Functions
//==============================================================================

inline float getTimerIntervalMs()
{
    return TIMER_INTERVAL_MS;
}

/**
 * @brief Easing function for smooth transitions (ease-in-out)
 */
inline float easeInOutQuad(float t)
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

/**
 * @brief Sinusoidal oscillation helper
 */
inline float oscillate(float time, float period, float minVal, float maxVal)
{
    float phase = std::fmod(time, period) / period;  // 0 to 1
    float sine = std::sin(phase * 2.0f * juce::MathConstants<float>::pi);
    float normalized = (sine + 1.0f) / 2.0f;  // 0 to 1
    return minVal + normalized * (maxVal - minVal);
}

//==============================================================================
// Icon Type Enum
//==============================================================================

enum class IconType
{
    Haas,
    Width,
    Phase,
    Output
};

//==============================================================================
// Icon Animator
//==============================================================================

/**
 * @brief Animates module icons with type-specific behaviors
 */
class IconAnimator
{
public:
    explicit IconAnimator(IconType type)
        : iconType(type),
          elapsedTime(0.0f),
          isCorrecting(false)
    {
    }

    /**
     * @brief Update animation state
     * @param deltaSeconds Time elapsed since last update in seconds
     */
    void update(float deltaSeconds)
    {
        elapsedTime += deltaSeconds;

        // Keep elapsed time bounded to prevent floating point issues
        float maxPeriod = std::max({HAAS_ICON_PERIOD, WIDTH_ICON_PERIOD,
                                    PHASE_ICON_PERIOD, OUTPUT_ICON_PERIOD}) * 10.0f;
        if (elapsedTime > maxPeriod)
        {
            elapsedTime = std::fmod(elapsedTime, maxPeriod);
        }
    }

    /**
     * @brief Get Y offset for HaasIcon wave motion
     */
    float getYOffset() const
    {
        if (iconType != IconType::Haas)
            return 0.0f;

        return oscillate(elapsedTime, HAAS_ICON_PERIOD,
                        -HAAS_ICON_AMPLITUDE, HAAS_ICON_AMPLITUDE);
    }

    /**
     * @brief Get scale factor for WidthIcon expand/contract
     */
    float getScale() const
    {
        if (iconType != IconType::Width)
            return 1.0f;

        return oscillate(elapsedTime, WIDTH_ICON_PERIOD,
                        WIDTH_ICON_MIN_SCALE, WIDTH_ICON_MAX_SCALE);
    }

    /**
     * @brief Get compression factor for PhaseIcon wave compression
     * Only animates when correcting is true
     */
    float getCompression() const
    {
        if (iconType != IconType::Phase)
            return 1.0f;

        if (!isCorrecting)
            return 1.0f;  // No compression when not correcting

        return oscillate(elapsedTime, PHASE_ICON_PERIOD,
                        PHASE_ICON_MIN_COMPRESSION, PHASE_ICON_MAX_COMPRESSION);
    }

    /**
     * @brief Get wave opacity for OutputIcon fade animation
     */
    float getWaveOpacity() const
    {
        if (iconType != IconType::Output)
            return 1.0f;

        return oscillate(elapsedTime, OUTPUT_ICON_PERIOD,
                        OUTPUT_ICON_MIN_OPACITY, OUTPUT_ICON_MAX_OPACITY);
    }

    /**
     * @brief Set correcting state (affects PhaseIcon animation)
     */
    void setCorrecting(bool correcting)
    {
        isCorrecting = correcting;
    }

    bool getCorrecting() const { return isCorrecting; }

    /**
     * @brief Reset animation to initial state
     */
    void reset()
    {
        elapsedTime = 0.0f;
    }

private:
    IconType iconType;
    float elapsedTime;
    bool isCorrecting;
};

//==============================================================================
// Badge Animator
//==============================================================================

/**
 * @brief Animates the "CORRECTING" badge pulse
 */
class BadgeAnimator
{
public:
    BadgeAnimator()
        : elapsedTime(0.0f)
    {
    }

    /**
     * @brief Update animation state
     * @param deltaSeconds Time elapsed since last update in seconds
     */
    void update(float deltaSeconds)
    {
        elapsedTime += deltaSeconds;

        // Keep bounded
        if (elapsedTime > BADGE_PERIOD * 10.0f)
        {
            elapsedTime = std::fmod(elapsedTime, BADGE_PERIOD * 10.0f);
        }
    }

    /**
     * @brief Get current opacity (oscillates between 0.5 and 1.0)
     */
    float getOpacity() const
    {
        return oscillate(elapsedTime, BADGE_PERIOD,
                        BADGE_MIN_OPACITY, BADGE_MAX_OPACITY);
    }

    /**
     * @brief Reset animation to initial state
     */
    void reset()
    {
        elapsedTime = 0.0f;
    }

private:
    float elapsedTime;
};

//==============================================================================
// Transition Animator
//==============================================================================

/**
 * @brief Animates value transitions with configurable duration
 * Used for border color and background tint fades
 */
class TransitionAnimator
{
public:
    explicit TransitionAnimator(float durationMs)
        : duration(durationMs),
          currentValue(0.0f),
          startValue(0.0f),
          targetValue(0.0f),
          elapsedMs(0.0f),
          isAnimating(false)
    {
    }

    /**
     * @brief Start a new transition
     * @param from Starting value
     * @param to Target value
     */
    void startTransition(float from, float to)
    {
        startValue = from;
        targetValue = to;
        currentValue = from;
        elapsedMs = 0.0f;
        isAnimating = true;
    }

    /**
     * @brief Update transition state
     * @param deltaMs Time elapsed since last update in milliseconds
     */
    void update(float deltaMs)
    {
        if (!isAnimating)
            return;

        elapsedMs += deltaMs;

        if (elapsedMs >= duration)
        {
            currentValue = targetValue;
            isAnimating = false;
        }
        else
        {
            float t = elapsedMs / duration;
            float eased = easeInOutQuad(t);
            currentValue = startValue + (targetValue - startValue) * eased;
        }
    }

    /**
     * @brief Get current animated value
     */
    float getCurrentValue() const { return currentValue; }

    /**
     * @brief Get transition duration in milliseconds
     */
    float getDurationMs() const { return duration; }

    /**
     * @brief Check if transition is in progress
     */
    bool isInProgress() const { return isAnimating; }

    /**
     * @brief Set current value immediately (no animation)
     */
    void setValue(float value)
    {
        currentValue = value;
        targetValue = value;
        isAnimating = false;
    }

    /**
     * @brief Reset to initial state
     */
    void reset()
    {
        currentValue = 0.0f;
        startValue = 0.0f;
        targetValue = 0.0f;
        elapsedMs = 0.0f;
        isAnimating = false;
    }

private:
    float duration;
    float currentValue;
    float startValue;
    float targetValue;
    float elapsedMs;
    bool isAnimating;
};

//==============================================================================
// Animation Controller
//==============================================================================

/**
 * @brief Central controller for all module animations
 * Provides a unified interface for managing animation state
 */
class AnimationController : public juce::Timer
{
public:
    AnimationController()
        : haasAnimator(IconType::Haas),
          widthAnimator(IconType::Width),
          phaseAnimator(IconType::Phase),
          outputAnimator(IconType::Output),
          borderTransition(BORDER_TRANSITION_MS),
          backgroundTransition(BACKGROUND_TINT_MS),
          lastUpdateTime(std::chrono::high_resolution_clock::now())
    {
    }

    ~AnimationController() override
    {
        stopTimer();
    }

    /**
     * @brief Start the animation timer at 30Hz
     */
    void startAnimations()
    {
        lastUpdateTime = std::chrono::high_resolution_clock::now();
        startTimer(static_cast<int>(TIMER_INTERVAL_MS));
    }

    /**
     * @brief Stop all animations
     */
    void stopAnimations()
    {
        stopTimer();
    }

    // Icon animators
    IconAnimator& getHaasAnimator() { return haasAnimator; }
    IconAnimator& getWidthAnimator() { return widthAnimator; }
    IconAnimator& getPhaseAnimator() { return phaseAnimator; }
    IconAnimator& getOutputAnimator() { return outputAnimator; }

    // Badge animator
    BadgeAnimator& getBadgeAnimator() { return badgeAnimator; }

    // Transition animators
    TransitionAnimator& getBorderTransition() { return borderTransition; }
    TransitionAnimator& getBackgroundTransition() { return backgroundTransition; }

    /**
     * @brief Set correcting state for phase module
     */
    void setCorrecting(bool correcting)
    {
        phaseAnimator.setCorrecting(correcting);
    }

    /**
     * @brief Callback to trigger repaint
     */
    std::function<void()> onAnimationUpdate;

private:
    void timerCallback() override
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<float>(now - lastUpdateTime).count();
        lastUpdateTime = now;

        // Update all animators
        haasAnimator.update(elapsed);
        widthAnimator.update(elapsed);
        phaseAnimator.update(elapsed);
        outputAnimator.update(elapsed);
        badgeAnimator.update(elapsed);

        // Update transitions (convert to ms)
        float elapsedMs = elapsed * 1000.0f;
        borderTransition.update(elapsedMs);
        backgroundTransition.update(elapsedMs);

        // Trigger repaint callback
        if (onAnimationUpdate)
            onAnimationUpdate();
    }

    IconAnimator haasAnimator;
    IconAnimator widthAnimator;
    IconAnimator phaseAnimator;
    IconAnimator outputAnimator;
    BadgeAnimator badgeAnimator;
    TransitionAnimator borderTransition;
    TransitionAnimator backgroundTransition;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdateTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnimationController)
};

} // namespace AnimationSystem
