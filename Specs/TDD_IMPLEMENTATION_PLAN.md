# HAAS FX Pro - TDD Implementation Plan

## Overview

This document provides a Test-Driven Development (TDD) implementation plan for the HAAS FX Pro UI redesign. Each iteration is fully functional and includes comprehensive unit tests before implementation.

**Testing Framework:** Catch2 (header-only, recommended for JUCE projects)
**Test Organization:** Tests mirror source structure in `Tests/` directory

---

## Test Infrastructure Setup

### Iteration 0: Test Framework Setup

**Goal:** Establish Catch2 testing infrastructure before any feature development.

#### Files to Create
```
Tests/
├── catch2/
│   └── catch.hpp              # Catch2 single-header (v3.x)
├── TestMain.cpp               # Catch2 test runner main
├── DSP/
│   └── (test files go here)
├── UI/
│   └── (test files go here)
└── CMakeLists.txt             # Test target configuration
```

#### CMakeLists.txt Addition
```cmake
# Test target
enable_testing()
add_executable(HaasAlignDelay_Tests
    Tests/TestMain.cpp
    # Test files will be added here
)
target_include_directories(HaasAlignDelay_Tests PRIVATE Tests/catch2)
target_link_libraries(HaasAlignDelay_Tests PRIVATE juce::juce_audio_basics)
add_test(NAME HaasAlignDelay_Tests COMMAND HaasAlignDelay_Tests)
```

#### Tests to Verify Setup
| Test ID | Test Name | Description |
|---------|-----------|-------------|
| T0.1 | `Sanity_TrueIsTrue` | Basic Catch2 sanity check |
| T0.2 | `Sanity_MathWorks` | Verify basic math operations |

#### Acceptance Criteria
- [ ] `ctest` runs and passes
- [ ] Test executable builds without errors
- [ ] Can add new test files and they are discovered

---

## Iteration 1: Color System & Constants

**Goal:** Implement the ReOrder Audio color palette as a testable module.

### Feature: ReOrderColors Namespace

**Behavior:**
- Provides all brand colors as `juce::Colour` constants
- Colors are organized by category (backgrounds, accents, text, status)
- Each module has a designated accent color
- Colors match the CLAUDE.md specification exactly

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T1.1 | `Colors_BackgroundDarkestValue` | `bgDarkest` equals `0xff0d0d0d` |
| T1.2 | `Colors_BackgroundDarkValue` | `bgDark` equals `0xff1a1a1a` |
| T1.3 | `Colors_BackgroundModuleValue` | `bgModule` equals `0xff141414` |
| T1.4 | `Colors_BorderValue` | `border` equals `0xff2a2a2a` |
| T1.5 | `Colors_BorderLightValue` | `borderLight` equals `0xff333333` |
| T1.6 | `Colors_AccentDelayValue` | `accentDelay` equals `0xffff66aa` (pink) |
| T1.7 | `Colors_AccentWidthValue` | `accentWidth` equals `0xff00d4ff` (cyan) |
| T1.8 | `Colors_AccentPhaseValue` | `accentPhase` equals `0xffff9933` (orange) |
| T1.9 | `Colors_AccentOutputValue` | `accentOutput` equals `0xff00ff88` (green) |
| T1.10 | `Colors_TextPrimaryValue` | `textPrimary` equals `0xffffffff` |
| T1.11 | `Colors_TextMutedValue` | `textMuted` equals `0xff888888` |
| T1.12 | `Colors_TextDimValue` | `textDim` equals `0xff666666` |
| T1.13 | `Colors_StatusGoodValue` | `statusGood` equals `0xff00ff88` |
| T1.14 | `Colors_StatusWarnValue` | `statusWarn` equals `0xffffcc00` |
| T1.15 | `Colors_StatusErrorValue` | `statusError` equals `0xffff3366` |
| T1.16 | `Colors_GetModuleAccent_Delay` | `getModuleAccent(ModuleType::Delay)` returns pink |
| T1.17 | `Colors_GetModuleAccent_Width` | `getModuleAccent(ModuleType::Width)` returns cyan |
| T1.18 | `Colors_GetModuleAccent_Phase` | `getModuleAccent(ModuleType::Phase)` returns orange |
| T1.19 | `Colors_GetModuleAccent_Output` | `getModuleAccent(ModuleType::Output)` returns green |
| T1.20 | `Colors_WithAlpha` | `withAlpha(accentDelay, 0.5f)` returns correct alpha value |

### Files
- `Source/UI/ReOrderColors.h` (implementation)
- `Tests/UI/ReOrderColorsTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 20 color tests pass
- [ ] Colors are accessible via namespace
- [ ] Helper function `getModuleAccent()` works correctly

---

## Iteration 2: DSP - Output Gain Stage

**Goal:** Add output gain parameter to HaasProcessor with smooth ramping.

### Feature: Output Gain Parameter

**Behavior:**
- Gain range: -12dB to +12dB
- Default: 0dB (unity gain)
- Applied after dry/wet mix stage
- Smoothed to prevent clicks (50ms ramp time)
- Bypassed when output module is disabled

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T2.1 | `OutputGain_DefaultIsUnity` | Default gain is 0dB (1.0 linear) |
| T2.2 | `OutputGain_MinusMax` | -12dB reduces signal by ~0.25x |
| T2.3 | `OutputGain_PlusMax` | +12dB increases signal by ~4x |
| T2.4 | `OutputGain_ZeroDb` | 0dB results in unity gain |
| T2.5 | `OutputGain_DbToLinear_Accuracy` | dB to linear conversion within 0.001 tolerance |
| T2.6 | `OutputGain_AppliedAfterMix` | Gain affects both wet and dry signals |
| T2.7 | `OutputGain_Smoothing_NoClicks` | Rapid changes don't produce samples > 1.5x expected |
| T2.8 | `OutputGain_Smoothing_RampTime` | 50ms ramp to target value |
| T2.9 | `OutputGain_BypassedWhenModuleOff` | Output bypass = true means no gain applied |
| T2.10 | `OutputGain_ProcessBlock_Stereo` | Both L/R channels gain-adjusted equally |

### Files
- `Source/DSP/HaasProcessor.h` (modify - add outputGain parameter)
- `Tests/DSP/OutputGainTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 10 output gain tests pass
- [ ] No audio clicks on gain changes
- [ ] Gain integrates with existing HaasProcessor

---

## Iteration 3: DSP - Width Low Cut Filter

**Goal:** Add high-pass filter to width processing to keep bass mono.

### Feature: Width Low Cut Filter

**Behavior:**
- Frequency range: 20Hz to 500Hz
- Default: 250Hz
- Applied to Side (S) channel in M/S processing
- 2nd-order Butterworth high-pass
- Only active when width module is enabled
- Below cutoff frequency, stereo width is reduced toward mono

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T3.1 | `WidthLowCut_DefaultFrequency` | Default is 250Hz |
| T3.2 | `WidthLowCut_RangeMin` | Accepts 20Hz |
| T3.3 | `WidthLowCut_RangeMax` | Accepts 500Hz |
| T3.4 | `WidthLowCut_BassAttenuation_50Hz` | 50Hz signal at cutoff 250Hz is attenuated >12dB |
| T3.5 | `WidthLowCut_HighsPass_1kHz` | 1kHz signal passes with <1dB attenuation |
| T3.6 | `WidthLowCut_AtCutoff_3dB` | Signal at cutoff frequency is -3dB |
| T3.7 | `WidthLowCut_SideChannelOnly` | Mid (M) channel is unaffected |
| T3.8 | `WidthLowCut_MonoBassResult` | Bass content has correlation near +1.0 |
| T3.9 | `WidthLowCut_BypassWhenModuleOff` | Width bypass disables low cut |
| T3.10 | `WidthLowCut_SmoothFreqChange` | Frequency changes don't cause clicks |
| T3.11 | `WidthLowCut_InteractionWithWidth` | Works correctly with width 100-200% |
| T3.12 | `WidthLowCut_FilterCoefficients` | Butterworth coefficients are correct |

### Files
- `Source/DSP/HaasProcessor.h` (modify - add BiquadFilter for width low cut)
- `Tests/DSP/WidthLowCutTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 width low cut tests pass
- [ ] Bass frequencies are mono-compatible
- [ ] High frequencies maintain stereo width

---

## Iteration 4: DSP - Correction Speed Parameter

**Goal:** Expose attack/release timing control in AutoPhaseCorrector.

### Feature: Correction Speed Parameter

**Behavior:**
- Range: 0-100%
- Default: 50%
- 0% = Slow (200ms attack, 800ms release) - gentle, musical
- 100% = Fast (10ms attack, 40ms release) - aggressive
- Affects how quickly phase correction engages and releases
- Does not affect analysis window size (stays 50ms)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T4.1 | `CorrectionSpeed_DefaultIs50Percent` | Default speed is 50% |
| T4.2 | `CorrectionSpeed_RangeMin` | Accepts 0% |
| T4.3 | `CorrectionSpeed_RangeMax` | Accepts 100% |
| T4.4 | `CorrectionSpeed_0Percent_AttackTime` | 0% speed = 200ms attack |
| T4.5 | `CorrectionSpeed_0Percent_ReleaseTime` | 0% speed = 800ms release |
| T4.6 | `CorrectionSpeed_100Percent_AttackTime` | 100% speed = 10ms attack |
| T4.7 | `CorrectionSpeed_100Percent_ReleaseTime` | 100% speed = 40ms release |
| T4.8 | `CorrectionSpeed_50Percent_Interpolated` | 50% speed = interpolated times |
| T4.9 | `CorrectionSpeed_SmoothTransition` | Speed changes don't cause jumps |
| T4.10 | `CorrectionSpeed_AffectsCorrectionEngage` | Slow speed = longer ramp to correction |
| T4.11 | `CorrectionSpeed_AffectsCorrectionRelease` | Fast speed = quick return after correction |
| T4.12 | `CorrectionSpeed_AnalysisWindowUnchanged` | 50ms analysis window stays constant |

### Files
- `Source/DSP/AutoPhaseCorrector.h` (modify - add speed parameter)
- `Tests/DSP/CorrectionSpeedTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 correction speed tests pass
- [ ] Musical correction at slow speeds
- [ ] Responsive correction at fast speeds

---

## Iteration 5: DSP - Module Bypass System

**Goal:** Implement per-module bypass with click-free transitions.

### Feature: Module Bypass Flags

**Behavior:**
- Four bypass flags: delay, width, phase, output
- Bypassed module passes audio unchanged
- Crossfade on bypass toggle (10ms) to prevent clicks
- Multiple bypasses can be active simultaneously
- Master bypass overrides all modules

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T5.1 | `ModuleBypass_Delay_Default` | Delay bypass default is false |
| T5.2 | `ModuleBypass_Width_Default` | Width bypass default is false |
| T5.3 | `ModuleBypass_Phase_Default` | Phase bypass default is false |
| T5.4 | `ModuleBypass_Output_Default` | Output bypass default is false |
| T5.5 | `ModuleBypass_Delay_NoProcessing` | Delay bypass = no delay applied |
| T5.6 | `ModuleBypass_Width_NoProcessing` | Width bypass = width stays 100% |
| T5.7 | `ModuleBypass_Phase_NoCorrection` | Phase bypass = no auto-correction |
| T5.8 | `ModuleBypass_Output_NoGain` | Output bypass = unity gain |
| T5.9 | `ModuleBypass_Crossfade_NoClicks` | Toggle doesn't produce samples > input |
| T5.10 | `ModuleBypass_Crossfade_Duration` | Crossfade completes in ~10ms |
| T5.11 | `ModuleBypass_Multiple_Simultaneous` | Can bypass multiple modules at once |
| T5.12 | `ModuleBypass_MasterBypass_Override` | Master bypass overrides all |
| T5.13 | `ModuleBypass_StatePreserved` | Bypass state persists across prepare() |
| T5.14 | `ModuleBypass_MetersContinue` | Metering works even when bypassed |

### Files
- `Source/DSP/HaasProcessor.h` (modify - add bypass flags and crossfade logic)
- `Tests/DSP/ModuleBypassTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 14 module bypass tests pass
- [ ] No audio artifacts on bypass toggle
- [ ] Each module bypasses independently

---

## Iteration 6: DSP - Delay Link Feature

**Goal:** Implement linked delay sliders with offset maintenance.

### Feature: Delay Link Mode

**Behavior:**
- When link enabled, adjusting one delay adjusts both
- Maintains relative offset between L and R
- If offset would push value out of range (0-50ms), clamps at boundary
- Link toggle does not change current values
- Works with both increasing and decreasing adjustments

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T6.1 | `DelayLink_DefaultOff` | Link is disabled by default |
| T6.2 | `DelayLink_EnableDoesNotChangeValues` | Enabling link preserves current L/R |
| T6.3 | `DelayLink_DisableDoesNotChangeValues` | Disabling link preserves current L/R |
| T6.4 | `DelayLink_AdjustLeft_MovesRight` | Increase L increases R by same amount |
| T6.5 | `DelayLink_AdjustRight_MovesLeft` | Increase R increases L by same amount |
| T6.6 | `DelayLink_MaintainsOffset` | 15ms offset maintained when adjusting |
| T6.7 | `DelayLink_ClampAtMax` | Right at 50ms, left increase stops right |
| T6.8 | `DelayLink_ClampAtMin` | Left at 0ms, right decrease stops left |
| T6.9 | `DelayLink_NegativeOffset` | Works when R > L |
| T6.10 | `DelayLink_ZeroOffset` | Works when L == R |
| T6.11 | `DelayLink_Unlinked_Independent` | Unlinked = independent adjustment |
| T6.12 | `DelayLink_RapidToggle` | Multiple toggles don't corrupt values |

### Files
- `Source/PluginProcessor.h` (modify - add link logic in parameterChanged)
- `Tests/DSP/DelayLinkTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 delay link tests pass
- [ ] Offset maintained correctly
- [ ] Boundary conditions handled gracefully

---

## Iteration 7: New Parameters Integration

**Goal:** Add all new parameters to PluginProcessor parameter layout.

### Feature: Extended Parameter Set

**New Parameters:**
| Parameter ID | Type | Range | Default |
|--------------|------|-------|---------|
| `delayLink` | Bool | - | false |
| `delayBypass` | Bool | - | false |
| `widthBypass` | Bool | - | false |
| `widthLowCut` | Float | 20-500 Hz | 250 |
| `phaseBypass` | Bool | - | false |
| `correctionSpeed` | Float | 0-100% | 50 |
| `outputBypass` | Bool | - | false |
| `outputGain` | Float | -12 to +12 dB | 0 |

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T7.1 | `Param_DelayLink_Exists` | Parameter tree contains `delayLink` |
| T7.2 | `Param_DelayLink_DefaultFalse` | Default value is false |
| T7.3 | `Param_DelayBypass_Exists` | Parameter tree contains `delayBypass` |
| T7.4 | `Param_WidthBypass_Exists` | Parameter tree contains `widthBypass` |
| T7.5 | `Param_WidthLowCut_Exists` | Parameter tree contains `widthLowCut` |
| T7.6 | `Param_WidthLowCut_Range` | Range is 20-500Hz |
| T7.7 | `Param_WidthLowCut_Default` | Default is 250Hz |
| T7.8 | `Param_PhaseBypass_Exists` | Parameter tree contains `phaseBypass` |
| T7.9 | `Param_CorrectionSpeed_Exists` | Parameter tree contains `correctionSpeed` |
| T7.10 | `Param_CorrectionSpeed_Range` | Range is 0-100% |
| T7.11 | `Param_CorrectionSpeed_Default` | Default is 50% |
| T7.12 | `Param_OutputBypass_Exists` | Parameter tree contains `outputBypass` |
| T7.13 | `Param_OutputGain_Exists` | Parameter tree contains `outputGain` |
| T7.14 | `Param_OutputGain_Range` | Range is -12 to +12 dB |
| T7.15 | `Param_OutputGain_Default` | Default is 0 dB |
| T7.16 | `Param_StateRestore_AllParams` | All parameters restore from state |
| T7.17 | `Param_StateSave_AllParams` | All parameters save to state |
| T7.18 | `Param_Automation_Supported` | Parameters are automatable |

### Files
- `Source/PluginProcessor.h` (modify)
- `Source/PluginProcessor.cpp` (modify)
- `Tests/PluginProcessorParamTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 18 parameter tests pass
- [ ] Parameters save/restore correctly
- [ ] DAW automation works for all parameters

---

## Iteration 8: UI - Vertical Level Meter Component

**Goal:** Create LED-style segmented vertical level meter.

### Feature: VerticalLevelMeter Component

**Behavior:**
- 24 segments, each 8×3px with 2px gap
- Color zones: green (0-60%), yellow (60-75%), red (75-100%)
- Smooth level decay (~50ms time constant)
- Peak hold indicator (2 second hold)
- Accepts 0.0-1.0 level input
- Active segments glow with accent color

### Unit Tests (Rendering Logic)

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T8.1 | `LevelMeter_SegmentCount` | Returns 24 segments |
| T8.2 | `LevelMeter_LevelZero_NoSegments` | Level 0.0 = 0 lit segments |
| T8.3 | `LevelMeter_LevelHalf_12Segments` | Level 0.5 = 12 lit segments |
| T8.4 | `LevelMeter_LevelFull_24Segments` | Level 1.0 = 24 lit segments |
| T8.5 | `LevelMeter_ColorZone_Green` | Segments 1-14 are green |
| T8.6 | `LevelMeter_ColorZone_Yellow` | Segments 15-18 are yellow |
| T8.7 | `LevelMeter_ColorZone_Red` | Segments 19-24 are red |
| T8.8 | `LevelMeter_Decay_TimeConstant` | Level decays to 50% in ~50ms |
| T8.9 | `LevelMeter_PeakHold_Duration` | Peak holds for 2 seconds |
| T8.10 | `LevelMeter_PeakHold_Position` | Peak indicator at highest point |
| T8.11 | `LevelMeter_ClampAboveOne` | Levels > 1.0 clamped to 1.0 |
| T8.12 | `LevelMeter_ClampBelowZero` | Levels < 0.0 clamped to 0.0 |
| T8.13 | `LevelMeter_SegmentDimensions` | Each segment is 8×3px |
| T8.14 | `LevelMeter_SegmentGap` | 2px gap between segments |

### Files
- `Source/UI/VerticalLevelMeter.h` (new)
- `Tests/UI/VerticalLevelMeterTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 14 level meter tests pass
- [ ] Visually matches specification
- [ ] Smooth decay animation

---

## Iteration 9: UI - Phase Correlation Meter Enhancement

**Goal:** Enhance correlation meter with threshold marker and correction indicator.

### Feature: Enhanced PhaseCorrelationMeter

**Behavior:**
- 200px × 24px dimensions
- Scale: -1 (left) to +1 (right)
- Gradient overlay: red → orange → yellow → green
- Draggable threshold marker (vertical line)
- Position indicator: 4×16px bar
- Color: red below threshold, green above
- "CORRECTING" badge when active (pulsing)
- Value readout below meter

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T9.1 | `CorrelationMeter_Dimensions` | 200×24px default size |
| T9.2 | `CorrelationMeter_ScaleRange` | -1.0 to +1.0 scale |
| T9.3 | `CorrelationMeter_ZeroPosition` | 0.0 = center of meter |
| T9.4 | `CorrelationMeter_NegativeOnePosition` | -1.0 = left edge |
| T9.5 | `CorrelationMeter_PositiveOnePosition` | +1.0 = right edge |
| T9.6 | `CorrelationMeter_ThresholdDefault` | Default threshold is 0.3 |
| T9.7 | `CorrelationMeter_ThresholdDraggable` | Threshold responds to mouse drag |
| T9.8 | `CorrelationMeter_ThresholdRange` | Threshold clamped 0.0-1.0 |
| T9.9 | `CorrelationMeter_IndicatorColor_BelowThreshold` | Red when below threshold |
| T9.10 | `CorrelationMeter_IndicatorColor_AboveThreshold` | Green when above threshold |
| T9.11 | `CorrelationMeter_CorrectingBadge_Hidden` | Badge hidden when not correcting |
| T9.12 | `CorrelationMeter_CorrectingBadge_Visible` | Badge visible when correcting |
| T9.13 | `CorrelationMeter_CorrectingBadge_Pulse` | Badge opacity oscillates |
| T9.14 | `CorrelationMeter_ValueReadout_Format` | Format: "+0.65" or "-0.32" |
| T9.15 | `CorrelationMeter_Smoothing` | ~100ms position smoothing |

### Files
- `Source/UI/PhaseCorrelationMeter.h` (new or modify existing)
- `Tests/UI/PhaseCorrelationMeterTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 15 correlation meter tests pass
- [ ] Threshold is draggable
- [ ] Correction badge pulses correctly

---

## Iteration 10: UI - Module Icons

**Goal:** Create SVG-style path icons for each module.

### Feature: ModuleIcons

**Behavior:**
- 60×60px canvas for each icon
- Stroke-based rendering (not filled)
- Uses module accent color
- Glow filter when module is active
- Optional animation state

**Icons:**
- **HaasIcon**: Two offset sine waves (delay visualization)
- **WidthIcon**: Expanding arrows from center point
- **PhaseIcon**: Concentric circles with crossing waves
- **OutputIcon**: Speaker with radiating sound waves

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T10.1 | `ModuleIcons_HaasIcon_PathExists` | HaasIcon returns valid path |
| T10.2 | `ModuleIcons_WidthIcon_PathExists` | WidthIcon returns valid path |
| T10.3 | `ModuleIcons_PhaseIcon_PathExists` | PhaseIcon returns valid path |
| T10.4 | `ModuleIcons_OutputIcon_PathExists` | OutputIcon returns valid path |
| T10.5 | `ModuleIcons_BoundsCheck` | All paths fit within 60×60 |
| T10.6 | `ModuleIcons_StrokeWidth` | Default stroke width is 2px |
| T10.7 | `ModuleIcons_HaasIcon_Color` | Uses accentDelay (pink) |
| T10.8 | `ModuleIcons_WidthIcon_Color` | Uses accentWidth (cyan) |
| T10.9 | `ModuleIcons_PhaseIcon_Color` | Uses accentPhase (orange) |
| T10.10 | `ModuleIcons_OutputIcon_Color` | Uses accentOutput (green) |
| T10.11 | `ModuleIcons_ActiveGlow` | Active state adds drop-shadow |
| T10.12 | `ModuleIcons_InactiveNoGlow` | Inactive state has no glow |

### Files
- `Source/UI/ModuleIcons.h` (new)
- `Tests/UI/ModuleIconsTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 icon tests pass
- [ ] Icons render at correct size
- [ ] Glow effect visible when active

---

## Iteration 11: UI - ReOrderLookAndFeel - Rotary Slider

**Goal:** Implement custom rotary slider (knob) rendering.

### Feature: Rotary Slider Styling

**Behavior:**
- 270° rotation range (-135° to +135°)
- Multi-layer glow effect on value arc
- 3D metallic knob body (radial gradient #3a3a3a to #1a1a1a)
- Center cap (35% of knob size)
- Indicator line on knob + dot on arc
- Color from slider properties (module accent)
- Drop shadow: `0 4px 12px rgba(0,0,0,0.5)`
- Arc glow: `0 0 20px {accent}22`

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T11.1 | `RotarySlider_RotationRange` | 270° total rotation |
| T11.2 | `RotarySlider_StartAngle` | Starts at -135° (7:30 position) |
| T11.3 | `RotarySlider_EndAngle` | Ends at +135° (4:30 position) |
| T11.4 | `RotarySlider_MinValue_StartAngle` | 0% = -135° |
| T11.5 | `RotarySlider_MaxValue_EndAngle` | 100% = +135° |
| T11.6 | `RotarySlider_MidValue_TopDead` | 50% ≠ top dead center (due to range) |
| T11.7 | `RotarySlider_KnobBodyGradient` | Uses radial gradient |
| T11.8 | `RotarySlider_CenterCapSize` | Cap is 35% of diameter |
| T11.9 | `RotarySlider_IndicatorWidth` | Indicator line is 2px wide |
| T11.10 | `RotarySlider_AccentColorFromProperty` | Gets accent from slider property |
| T11.11 | `RotarySlider_ArcGlowPresent` | Glow effect on arc |
| T11.12 | `RotarySlider_DropShadowPresent` | Drop shadow rendered |
| T11.13 | `RotarySlider_BorderColor` | 2px border in #333 |

### Files
- `Source/UI/ReOrderLookAndFeel.h` (new)
- `Source/UI/ReOrderLookAndFeel.cpp` (new)
- `Tests/UI/ReOrderLookAndFeelTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 13 rotary slider tests pass
- [ ] Knob visually matches specification
- [ ] Accent color properly applied

---

## Iteration 12: UI - ReOrderLookAndFeel - Linear Sliders

**Goal:** Implement vertical and horizontal slider rendering.

### Feature: Linear Slider Styling

**Vertical Slider:**
- Track: 12px wide, #1a1a1a background, 1px #333 border, rounded
- Fill: 6px centered, accent gradient with glow, from bottom up
- Handle: 20×16px, metallic gradient (#555 to #333), grip lines

**Horizontal Slider:**
- Track: 8px tall
- Fill from left
- Handle: 16px circular or rectangular

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T12.1 | `VerticalSlider_TrackWidth` | Track is 12px wide |
| T12.2 | `VerticalSlider_FillWidth` | Fill is 6px centered |
| T12.3 | `VerticalSlider_FillFromBottom` | Fill starts from bottom |
| T12.4 | `VerticalSlider_HandleSize` | Handle is 20×16px |
| T12.5 | `VerticalSlider_HandleGradient` | Uses metallic gradient |
| T12.6 | `VerticalSlider_GlowOnFill` | Fill has accent glow |
| T12.7 | `HorizontalSlider_TrackHeight` | Track is 8px tall |
| T12.8 | `HorizontalSlider_FillFromLeft` | Fill starts from left |
| T12.9 | `HorizontalSlider_ThumbSize` | Thumb is 16px |
| T12.10 | `LinearSlider_AccentFromProperty` | Gets accent from property |
| T12.11 | `LinearSlider_InsetShadow` | Track has inset shadow |
| T12.12 | `LinearSlider_RoundedCorners` | 6px border radius |

### Files
- `Source/UI/ReOrderLookAndFeel.cpp` (modify - add linear slider drawing)
- `Tests/UI/LinearSliderLookAndFeelTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 linear slider tests pass
- [ ] Both orientations render correctly
- [ ] Handles have 3D appearance

---

## Iteration 13: UI - ReOrderLookAndFeel - Buttons

**Goal:** Implement power, toggle, and bypass button rendering.

### Feature: Button Styling

**Power Button:**
- 20×20px circle
- Border: 2px, accent color (on) or #444 (off)
- Fill: accent @ 20% opacity (on) or transparent (off)

**Toggle Button (LINK, AUTO FIX):**
- Rectangular, rounded corners (4-6px radius)
- Off: #1a1a1a bg, #333 border, #666 text
- On: Accent gradient fill, accent border, glow, contrasting text

**Bypass Button:**
- Red (#ff3366) when bypassed (plugin active)
- Gray when inactive (plugin processing)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T13.1 | `PowerButton_Size` | 20×20px |
| T13.2 | `PowerButton_Off_BorderColor` | #444 when off |
| T13.3 | `PowerButton_On_BorderColor` | Accent color when on |
| T13.4 | `PowerButton_Off_FillTransparent` | Transparent fill when off |
| T13.5 | `PowerButton_On_FillTinted` | 20% accent fill when on |
| T13.6 | `ToggleButton_Off_Background` | #1a1a1a background |
| T13.7 | `ToggleButton_Off_TextColor` | #666 text |
| T13.8 | `ToggleButton_On_GradientFill` | Gradient in accent color |
| T13.9 | `ToggleButton_On_Glow` | Has glow effect |
| T13.10 | `ToggleButton_BorderRadius` | 4-6px rounded corners |
| T13.11 | `BypassButton_Active_GrayColor` | Gray when plugin processing |
| T13.12 | `BypassButton_Bypassed_RedColor` | Red when bypassed |
| T13.13 | `BypassButton_Bypassed_Glow` | Glow when bypassed |

### Files
- `Source/UI/ReOrderLookAndFeel.cpp` (modify - add button drawing)
- `Tests/UI/ButtonLookAndFeelTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 13 button tests pass
- [ ] Power buttons show correct states
- [ ] Toggle buttons have proper on/off styling

---

## Iteration 14: UI - Module Component Base Class

**Goal:** Create reusable base component for all module panels.

### Feature: ModuleComponent Base Class

**Behavior:**
- Standard header: power button, title (uppercase), optional badge
- Icon area: 60-80px for module icon
- Controls area: flexible layout for knobs/sliders
- Accent color property
- Active/inactive state with opacity reduction (30-40% when inactive)
- Background: #141414, border: 1px #2a2a2a, radius: 10px
- Active glow border when enabled

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T14.1 | `ModuleComponent_BackgroundColor` | #141414 background |
| T14.2 | `ModuleComponent_BorderColor` | #2a2a2a border |
| T14.3 | `ModuleComponent_BorderRadius` | 10px rounded corners |
| T14.4 | `ModuleComponent_HeaderHeight` | 40px header area |
| T14.5 | `ModuleComponent_PowerButtonInHeader` | Power button at left of header |
| T14.6 | `ModuleComponent_TitleUppercase` | Title is uppercase |
| T14.7 | `ModuleComponent_TitleColor` | Title uses accent color |
| T14.8 | `ModuleComponent_InactiveOpacity` | 30-40% opacity when inactive |
| T14.9 | `ModuleComponent_ActiveGlow` | Glow border when active |
| T14.10 | `ModuleComponent_AccentColorProperty` | Can set accent color |
| T14.11 | `ModuleComponent_IconAreaExists` | Has icon area section |
| T14.12 | `ModuleComponent_ControlsAreaExists` | Has controls area section |

### Files
- `Source/UI/ModuleComponent.h` (new)
- `Tests/UI/ModuleComponentTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 base component tests pass
- [ ] Reusable for all 4 modules
- [ ] State changes update visual appearance

---

## Iteration 15: UI - Delay Module

**Goal:** Implement the Delay module panel with linked sliders.

### Feature: DelayModule Component

**Layout:**
```
[●] ──── DELAY ────
      [Haas Icon]
   [L Slider] [R Slider]
     0.0ms      15.0ms
     LEFT       RIGHT
       [LINK]
```

**Behavior:**
- Extends ModuleComponent
- Two vertical sliders (0-50ms each)
- LINK toggle button
- Link logic: dragging one slider adjusts both, maintaining offset
- Pink accent color (#ff66aa)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T15.1 | `DelayModule_AccentColor` | Uses pink (#ff66aa) |
| T15.2 | `DelayModule_HasLeftSlider` | Contains left delay slider |
| T15.3 | `DelayModule_HasRightSlider` | Contains right delay slider |
| T15.4 | `DelayModule_SliderRange` | 0-50ms range |
| T15.5 | `DelayModule_HasLinkButton` | Contains LINK toggle |
| T15.6 | `DelayModule_HasIcon` | Contains HaasIcon |
| T15.7 | `DelayModule_LinkOff_Independent` | Sliders move independently |
| T15.8 | `DelayModule_LinkOn_Coupled` | Sliders move together |
| T15.9 | `DelayModule_LinkOn_MaintainsOffset` | Offset preserved |
| T15.10 | `DelayModule_ValueDisplay_Format` | "0.0ms" format |
| T15.11 | `DelayModule_PowerButton_Bypass` | Power button bypasses delay |
| T15.12 | `DelayModule_ParameterAttachment` | Connected to processor params |

### Files
- `Source/UI/DelayModule.h` (new)
- `Tests/UI/DelayModuleTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 delay module tests pass
- [ ] Link functionality works correctly
- [ ] Visual styling matches specification

---

## Iteration 16: UI - Width Module

**Goal:** Implement the Width module panel with low cut and phase buttons.

### Feature: WidthModule Component

**Layout:**
```
[●] ──── WIDTH ────
      [Width Icon]
     [Width Knob]
        150%
    STEREO WIDTH

  LOW CUT        250Hz
  [━━━━━●━━━━━━━━━━━]

    [⦿L]    [⦿R]
   PHASE L  PHASE R
```

**Behavior:**
- Width knob: 0-200%
- Low cut slider: 20-500Hz horizontal
- Phase invert buttons for L/R
- Cyan accent color (#00d4ff)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T16.1 | `WidthModule_AccentColor` | Uses cyan (#00d4ff) |
| T16.2 | `WidthModule_HasWidthKnob` | Contains width knob |
| T16.3 | `WidthModule_WidthRange` | 0-200% range |
| T16.4 | `WidthModule_HasLowCutSlider` | Contains low cut slider |
| T16.5 | `WidthModule_LowCutRange` | 20-500Hz range |
| T16.6 | `WidthModule_HasPhaseLButton` | Contains Phase L button |
| T16.7 | `WidthModule_HasPhaseRButton` | Contains Phase R button |
| T16.8 | `WidthModule_HasIcon` | Contains WidthIcon |
| T16.9 | `WidthModule_WidthValueFormat` | "150%" format |
| T16.10 | `WidthModule_LowCutValueFormat` | "250Hz" format |
| T16.11 | `WidthModule_PowerButton_Bypass` | Power button bypasses width |
| T16.12 | `WidthModule_ParameterAttachment` | Connected to processor params |

### Files
- `Source/UI/WidthModule.h` (new)
- `Tests/UI/WidthModuleTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 width module tests pass
- [ ] Low cut slider works horizontally
- [ ] Phase buttons toggle correctly

---

## Iteration 17: UI - Phase Module

**Goal:** Implement the Auto Phase module with correlation meter and correction indicator.

### Feature: PhaseModule Component

**Layout:**
```
[●] ── AUTO PHASE ── [EXCLUSIVE]
        [Phase Icon]

  -1        0        +1
  [━━━━━━━|━━━━●━━━━━━]
         0.65

      [AUTO FIX ON]

  [Threshold]  [Speed]
     0.30        50%
   THRESHOLD    SPEED
```

**Behavior:**
- Correlation meter with threshold
- AUTO FIX toggle
- Threshold knob (0-1.0)
- Speed knob (0-100%)
- "CORRECTING" badge when active (pulse animation)
- Orange accent color (#ff9933)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T17.1 | `PhaseModule_AccentColor` | Uses orange (#ff9933) |
| T17.2 | `PhaseModule_HasCorrelationMeter` | Contains correlation meter |
| T17.3 | `PhaseModule_HasAutoFixToggle` | Contains AUTO FIX button |
| T17.4 | `PhaseModule_HasThresholdKnob` | Contains threshold knob |
| T17.5 | `PhaseModule_ThresholdRange` | 0-1.0 range |
| T17.6 | `PhaseModule_HasSpeedKnob` | Contains speed knob |
| T17.7 | `PhaseModule_SpeedRange` | 0-100% range |
| T17.8 | `PhaseModule_HasIcon` | Contains PhaseIcon |
| T17.9 | `PhaseModule_HasExclusiveBadge` | Shows "EXCLUSIVE" badge |
| T17.10 | `PhaseModule_CorrectingBadge_Hidden` | Badge hidden when not correcting |
| T17.11 | `PhaseModule_CorrectingBadge_Visible` | Badge visible when correcting |
| T17.12 | `PhaseModule_CorrectingBadge_Pulses` | Badge opacity animates |
| T17.13 | `PhaseModule_ActiveGlow` | Orange glow when correcting |
| T17.14 | `PhaseModule_ParameterAttachment` | Connected to processor params |

### Files
- `Source/UI/PhaseModule.h` (new)
- `Tests/UI/PhaseModuleTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 14 phase module tests pass
- [ ] Correlation meter shows real-time data
- [ ] Correction indicator pulses correctly

---

## Iteration 18: UI - Output Module

**Goal:** Implement the Output module with gain and mix controls.

### Feature: OutputModule Component

**Layout:**
```
[●] ──── OUTPUT ────
      [Output Icon]

      [Gain Knob]
        +0.0dB
         GAIN

  DRY/WET          100%
  [━━━━━━━━━━━━━━━━━●]
```

**Behavior:**
- Gain knob: -12 to +12 dB
- Dry/wet horizontal slider: 0-100%
- Green accent color (#00ff88)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T18.1 | `OutputModule_AccentColor` | Uses green (#00ff88) |
| T18.2 | `OutputModule_HasGainKnob` | Contains gain knob |
| T18.3 | `OutputModule_GainRange` | -12 to +12 dB range |
| T18.4 | `OutputModule_HasMixSlider` | Contains dry/wet slider |
| T18.5 | `OutputModule_MixRange` | 0-100% range |
| T18.6 | `OutputModule_HasIcon` | Contains OutputIcon |
| T18.7 | `OutputModule_GainValueFormat` | "+0.0dB" format |
| T18.8 | `OutputModule_MixValueFormat` | "100%" format |
| T18.9 | `OutputModule_PowerButton_Bypass` | Power button bypasses output |
| T18.10 | `OutputModule_ParameterAttachment` | Connected to processor params |

### Files
- `Source/UI/OutputModule.h` (new)
- `Tests/UI/OutputModuleTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 10 output module tests pass
- [ ] Gain knob shows dB values correctly
- [ ] Mix slider works horizontally

---

## Iteration 19: UI - Header & Footer Components

**Goal:** Implement header with logo/preset selector and footer with branding.

### Feature: HeaderComponent

**Behavior:**
- Logo: Circle + crosshairs + "HAASFX" gradient text + "PRO" badge
- Preset selector (center): arrow buttons + preset name dropdown
- Bypass button (right): red when bypassed
- Background: gradient #222 to #1a1a1a
- Height: 48px

### Feature: FooterComponent

**Behavior:**
- Centered "REORDER AUDIO" text
- "REORDER" in #444, "AUDIO" in cyan (#00d4ff)
- Background: #0d0d0d
- Height: 40px

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T19.1 | `Header_Height` | 48px tall |
| T19.2 | `Header_BackgroundGradient` | #222 to #1a1a1a gradient |
| T19.3 | `Header_HasLogo` | Contains logo component |
| T19.4 | `Header_HasPresetSelector` | Contains preset selector |
| T19.5 | `Header_HasBypassButton` | Contains bypass button |
| T19.6 | `Header_BypassButton_RedWhenBypassed` | Red color when bypassed |
| T19.7 | `Header_PresetSelector_Centered` | Preset selector is centered |
| T19.8 | `Footer_Height` | 40px tall |
| T19.9 | `Footer_BackgroundColor` | #0d0d0d |
| T19.10 | `Footer_ReorderColor` | "REORDER" in #444 |
| T19.11 | `Footer_AudioColor` | "AUDIO" in cyan |
| T19.12 | `Footer_TextCentered` | Text is centered |

### Files
- `Source/UI/HeaderComponent.h` (new)
- `Source/UI/FooterComponent.h` (new)
- `Tests/UI/HeaderFooterTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 header/footer tests pass
- [ ] Logo renders correctly
- [ ] Branding matches specification

---

## Iteration 20: UI - Meter Strip Component

**Goal:** Implement input/output meter strip panels.

### Feature: MeterStrip Component

**Behavior:**
- 50px wide panel
- Vertical label (rotated 180° for input, 0° for output)
- L/R VerticalLevelMeter pair
- dB readout at bottom (-∞ to 0dB)
- Background: #0d0d0d
- Border: 1px #2a2a2a, 8px radius

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T20.1 | `MeterStrip_Width` | 50px wide |
| T20.2 | `MeterStrip_BackgroundColor` | #0d0d0d |
| T20.3 | `MeterStrip_BorderColor` | #2a2a2a |
| T20.4 | `MeterStrip_BorderRadius` | 8px |
| T20.5 | `MeterStrip_HasLeftMeter` | Contains L meter |
| T20.6 | `MeterStrip_HasRightMeter` | Contains R meter |
| T20.7 | `MeterStrip_HasLabel` | Contains rotated label |
| T20.8 | `MeterStrip_HasDbReadout` | Contains dB readout |
| T20.9 | `MeterStrip_InputLabelRotation` | Input label rotated 180° |
| T20.10 | `MeterStrip_OutputLabelRotation` | Output label rotated 0° |
| T20.11 | `MeterStrip_DbFormat` | "-12dB" or "-∞" format |
| T20.12 | `MeterStrip_UpdatesFromProcessor` | Responds to processor metering |

### Files
- `Source/UI/MeterStrip.h` (new)
- `Tests/UI/MeterStripTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 meter strip tests pass
- [ ] Meters update from audio levels
- [ ] Labels are correctly rotated

---

## Iteration 21: UI - Main Editor Layout

**Goal:** Restructure PluginEditor with new layout.

### Feature: New Editor Layout

**Structure:**
```
┌──────────────────────────────────────────────────────────┐
│ HEADER (48px)                                            │
├────┬─────────┬─────────┬─────────┬─────────┬────────────┤
│ IN │  DELAY  │  WIDTH  │  PHASE  │ OUTPUT  │    OUT     │
│ 50 │  flex   │  flex   │  flex   │  flex   │    50      │
├────┴─────────┴─────────┴─────────┴─────────┴────────────┤
│ FOOTER (40px)                                            │
└──────────────────────────────────────────────────────────┘
```

**Dimensions:** 1000 × 625px (fixed for initial implementation)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T21.1 | `Editor_DefaultSize` | 1000×625px |
| T21.2 | `Editor_HasHeader` | Contains HeaderComponent |
| T21.3 | `Editor_HasFooter` | Contains FooterComponent |
| T21.4 | `Editor_HasInputMeter` | Contains input MeterStrip |
| T21.5 | `Editor_HasOutputMeter` | Contains output MeterStrip |
| T21.6 | `Editor_HasDelayModule` | Contains DelayModule |
| T21.7 | `Editor_HasWidthModule` | Contains WidthModule |
| T21.8 | `Editor_HasPhaseModule` | Contains PhaseModule |
| T21.9 | `Editor_HasOutputModule` | Contains OutputModule |
| T21.10 | `Editor_ModulesInOrder` | Modules ordered L to R |
| T21.11 | `Editor_MeterStripPositions` | Meters at far left/right |
| T21.12 | `Editor_UsesReOrderLookAndFeel` | Uses new LookAndFeel |
| T21.13 | `Editor_TimerForMeters` | Timer updates meters |
| T21.14 | `Editor_ParameterConnections` | All params connected |

### Files
- `Source/PluginEditor.h` (rewrite)
- `Source/PluginEditor.cpp` (rewrite)
- `Tests/UI/PluginEditorTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 14 editor tests pass
- [ ] Layout matches specification
- [ ] All components visible and functional

---

## Iteration 22: Preset System - PresetManager

**Goal:** Implement preset file management.

### Feature: PresetManager

**Behavior:**
- Scans preset directory on startup
- Loads/saves `.preset` files (XML format)
- Factory presets embedded in binary
- User preset directory (platform-specific paths)
- Preset data structure matching all parameters

**File Format:**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Clean Stereo">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <!-- ... all parameters ... -->
  </Parameters>
</HaasPreset>
```

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T22.1 | `PresetManager_FactoryPresetsExist` | Has at least 6 factory presets |
| T22.2 | `PresetManager_LoadPreset_ValidFile` | Loads valid preset file |
| T22.3 | `PresetManager_LoadPreset_InvalidFile` | Returns error for invalid file |
| T22.4 | `PresetManager_SavePreset_CreatesFile` | Creates preset file |
| T22.5 | `PresetManager_SavePreset_ValidXML` | File is valid XML |
| T22.6 | `PresetManager_SavePreset_AllParams` | All parameters included |
| T22.7 | `PresetManager_GetPresetList` | Returns list of available presets |
| T22.8 | `PresetManager_GetPresetList_IncludesFactory` | List includes factory presets |
| T22.9 | `PresetManager_GetPresetList_IncludesUser` | List includes user presets |
| T22.10 | `PresetManager_DeletePreset` | Removes preset file |
| T22.11 | `PresetManager_RenamePreset` | Renames preset file |
| T22.12 | `PresetManager_PresetDirectory_macOS` | Correct macOS path |
| T22.13 | `PresetManager_PresetVersion` | Version attribute present |
| T22.14 | `PresetManager_ModifiedIndicator` | Tracks unsaved changes |

### Files
- `Source/Presets/PresetManager.h` (new)
- `Source/Presets/PresetManager.cpp` (new)
- `Source/Presets/FactoryPresets.h` (new)
- `Tests/Presets/PresetManagerTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 14 preset manager tests pass
- [ ] Factory presets load correctly
- [ ] User presets save/load correctly

---

## Iteration 23: Preset System - UI Integration

**Goal:** Integrate preset selector with PresetManager.

### Feature: PresetSelector Component

**Behavior:**
- Left/right arrow buttons for navigation
- Central dropdown showing preset name
- Click dropdown to open full list
- Shows [Factory] and [User] sections
- Modified indicator (*) for unsaved changes
- Save/Save As via right-click menu

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T23.1 | `PresetSelector_HasLeftArrow` | Contains left navigation button |
| T23.2 | `PresetSelector_HasRightArrow` | Contains right navigation button |
| T23.3 | `PresetSelector_HasDropdown` | Contains preset dropdown |
| T23.4 | `PresetSelector_LeftArrow_PreviousPreset` | Left goes to previous |
| T23.5 | `PresetSelector_RightArrow_NextPreset` | Right goes to next |
| T23.6 | `PresetSelector_LeftArrow_WrapsAround` | Wraps at beginning |
| T23.7 | `PresetSelector_RightArrow_WrapsAround` | Wraps at end |
| T23.8 | `PresetSelector_Dropdown_ShowsList` | Click shows preset list |
| T23.9 | `PresetSelector_ShowsModifiedIndicator` | Shows * when modified |
| T23.10 | `PresetSelector_RightClick_ShowsMenu` | Right-click shows context menu |
| T23.11 | `PresetSelector_LoadPreset_UpdatesParams` | Loading updates all params |
| T23.12 | `PresetSelector_MinWidth` | At least 180px wide |

### Files
- `Source/UI/PresetSelector.h` (new)
- `Tests/UI/PresetSelectorTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 preset selector tests pass
- [ ] Navigation works correctly
- [ ] Presets load and apply properly

---

## Iteration 24: Animation System

**Goal:** Add animations for icons, correction state, and controls.

### Feature: Module Icon Animations

**Behavior:**
- HaasIcon: Wave motion (translateY oscillation, 2s period)
- WidthIcon: Expand/contract arrows (scale, 1.5s period)
- PhaseIcon: Wave compression when correcting
- OutputIcon: Sound wave fade animation

### Feature: Correction State Animation

**Behavior:**
- Module border color transition (300ms)
- Background tint fade (200ms)
- "CORRECTING" badge pulse (1s cycle, opacity 1↔0.5)

### Unit Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T24.1 | `Animation_HaasIcon_Oscillates` | Y position changes over time |
| T24.2 | `Animation_HaasIcon_Period` | 2 second period |
| T24.3 | `Animation_WidthIcon_Scales` | Scale changes over time |
| T24.4 | `Animation_PhaseIcon_Active` | Animation when correcting |
| T24.5 | `Animation_PhaseIcon_Inactive` | No animation when not correcting |
| T24.6 | `Animation_OutputIcon_Fades` | Opacity changes over time |
| T24.7 | `Animation_CorrectingBadge_Pulse` | Opacity oscillates |
| T24.8 | `Animation_CorrectingBadge_Period` | 1 second period |
| T24.9 | `Animation_BorderTransition_Duration` | 300ms transition |
| T24.10 | `Animation_BackgroundTint_Duration` | 200ms fade |
| T24.11 | `Animation_Timer_30Hz` | Animations update at 30Hz |
| T24.12 | `Animation_LowCPU` | Animation overhead < 1% CPU |

### Files
- `Source/UI/AnimationSystem.h` (new)
- `Tests/UI/AnimationSystemTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 animation tests pass
- [ ] Animations are smooth at 30Hz
- [ ] CPU overhead is minimal

---

## Iteration 25: Polish & Integration Testing

**Goal:** Final integration testing and polish.

### Integration Tests

| Test ID | Test Name | Expected Behavior |
|---------|-----------|-------------------|
| T25.1 | `Integration_FullSignalPath` | Audio passes through all modules |
| T25.2 | `Integration_AllParamsAutomatable` | DAW automation for all params |
| T25.3 | `Integration_StateRestore` | Full state restores correctly |
| T25.4 | `Integration_PresetLoad_RestoresUI` | UI updates when preset loads |
| T25.5 | `Integration_ModuleBypass_NoClicks` | No clicks on any bypass |
| T25.6 | `Integration_LinkMode_Works` | Delay link works end-to-end |
| T25.7 | `Integration_Metering_Accurate` | Meters match actual levels |
| T25.8 | `Integration_Correlation_Accurate` | Correlation matches actual phase |
| T25.9 | `Integration_AutoCorrection_Works` | Auto phase reduces correlation |
| T25.10 | `Integration_60FPS_UI` | UI renders at 60fps |
| T25.11 | `Integration_NoMemoryLeaks` | No leaks on editor open/close |
| T25.12 | `Integration_ValidatesInDAWs` | Works in Logic, Ableton, PT |

### Files
- `Tests/Integration/FullPluginTest.cpp` (tests)

### Acceptance Criteria
- [ ] All 12 integration tests pass
- [ ] Plugin validates in all target DAWs
- [ ] No audio glitches or visual artifacts

---

## Test Summary

### Total Unit Tests by Iteration

| Iteration | Feature | Test Count |
|-----------|---------|------------|
| 0 | Test Framework Setup | 2 |
| 1 | Color System | 20 |
| 2 | Output Gain | 10 |
| 3 | Width Low Cut | 12 |
| 4 | Correction Speed | 12 |
| 5 | Module Bypass | 14 |
| 6 | Delay Link | 12 |
| 7 | New Parameters | 18 |
| 8 | Vertical Level Meter | 14 |
| 9 | Correlation Meter | 15 |
| 10 | Module Icons | 12 |
| 11 | Rotary Slider LnF | 13 |
| 12 | Linear Slider LnF | 12 |
| 13 | Button LnF | 13 |
| 14 | Module Component | 12 |
| 15 | Delay Module | 12 |
| 16 | Width Module | 12 |
| 17 | Phase Module | 14 |
| 18 | Output Module | 10 |
| 19 | Header & Footer | 12 |
| 20 | Meter Strip | 12 |
| 21 | Main Editor | 14 |
| 22 | Preset Manager | 14 |
| 23 | Preset Selector | 12 |
| 24 | Animation System | 12 |
| 25 | Integration | 12 |
| **TOTAL** | | **319** |

---

## File Structure Summary

### New Files to Create

```
Tests/
├── catch2/
│   └── catch.hpp
├── TestMain.cpp
├── DSP/
│   ├── OutputGainTest.cpp
│   ├── WidthLowCutTest.cpp
│   ├── CorrectionSpeedTest.cpp
│   ├── ModuleBypassTest.cpp
│   └── DelayLinkTest.cpp
├── UI/
│   ├── ReOrderColorsTest.cpp
│   ├── VerticalLevelMeterTest.cpp
│   ├── PhaseCorrelationMeterTest.cpp
│   ├── ModuleIconsTest.cpp
│   ├── ReOrderLookAndFeelTest.cpp
│   ├── LinearSliderLookAndFeelTest.cpp
│   ├── ButtonLookAndFeelTest.cpp
│   ├── ModuleComponentTest.cpp
│   ├── DelayModuleTest.cpp
│   ├── WidthModuleTest.cpp
│   ├── PhaseModuleTest.cpp
│   ├── OutputModuleTest.cpp
│   ├── HeaderFooterTest.cpp
│   ├── MeterStripTest.cpp
│   ├── PluginEditorTest.cpp
│   ├── PresetSelectorTest.cpp
│   └── AnimationSystemTest.cpp
├── Presets/
│   └── PresetManagerTest.cpp
├── Integration/
│   └── FullPluginTest.cpp
└── PluginProcessorParamTest.cpp

Source/
├── UI/
│   ├── ReOrderColors.h
│   ├── ReOrderLookAndFeel.h
│   ├── ReOrderLookAndFeel.cpp
│   ├── VerticalLevelMeter.h
│   ├── PhaseCorrelationMeter.h
│   ├── ModuleIcons.h
│   ├── ModuleComponent.h
│   ├── DelayModule.h
│   ├── WidthModule.h
│   ├── PhaseModule.h
│   ├── OutputModule.h
│   ├── HeaderComponent.h
│   ├── FooterComponent.h
│   ├── MeterStrip.h
│   ├── PresetSelector.h
│   └── AnimationSystem.h
└── Presets/
    ├── PresetManager.h
    ├── PresetManager.cpp
    └── FactoryPresets.h
```

### Files to Modify

```
Source/
├── PluginProcessor.h
├── PluginProcessor.cpp
├── PluginEditor.h
├── PluginEditor.cpp
├── DSP/
│   ├── HaasProcessor.h
│   └── AutoPhaseCorrector.h
CMakeLists.txt
```

---

## TDD Workflow

For each iteration:

1. **Write Tests First**
   - Create test file with all test cases
   - Tests should initially fail (RED)

2. **Implement Minimum Code**
   - Write just enough code to pass tests
   - Run tests frequently

3. **Refactor**
   - Clean up implementation
   - Ensure tests still pass (GREEN)

4. **Document**
   - Update any relevant documentation
   - Mark iteration as complete

5. **Move to Next Iteration**
   - Only proceed when all tests pass
   - Each iteration builds on previous

---

## Success Criteria

1. All 319 unit tests pass
2. All 12 integration tests pass
3. Plugin validates in Logic Pro, Ableton Live, Pro Tools
4. UI matches ReOrder Audio design system specification
5. No audio clicks/pops on any parameter change
6. CPU usage < 5% on modern hardware
7. Memory usage stable (no leaks)
8. Preset system works correctly

---

*TDD Implementation Plan Version: 1.0*
*Created: December 2024*
*Total Iterations: 26*
*Total Unit Tests: 319*
*Target: HAAS FX Pro v2.0*
