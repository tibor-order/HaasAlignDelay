# HAAS FX Pro - UI Implementation Plan

## Overview

This plan details the implementation of the new HAAS FX Pro UI design, transforming the current "XVOX Pro" style into the ReOrder Audio design system with modular layout, new controls, and enhanced features.

**Current State:** Working plugin with full DSP, purple-themed UI, horizontal layout
**Target State:** 1000×625px modular UI with pink/cyan/orange/green color scheme per module

---

## Phase 1: Foundation & Architecture

### 1.1 New Parameters (PluginProcessor)

Add these new parameters to `createParameterLayout()`:

| Parameter ID | Type | Range | Default | Description |
|--------------|------|-------|---------|-------------|
| `delayLink` | Bool | - | false | Link L/R delay sliders |
| `delayBypass` | Bool | - | false | Bypass delay module |
| `widthBypass` | Bool | - | false | Bypass width module |
| `widthLowCut` | Float | 20-500 Hz | 250 | Low cut frequency for width processing |
| `phaseBypass` | Bool | - | false | Bypass phase module |
| `correctionSpeed` | Float | 0-100% | 50 | Attack/release speed for auto-correction |
| `outputBypass` | Bool | - | false | Bypass output module |
| `outputGain` | Float | -12 to +12 dB | 0 | Makeup gain |

**Files to modify:**
- `Source/PluginProcessor.h` - Add parameter pointers
- `Source/PluginProcessor.cpp` - Add to parameter layout, add smoothed values

### 1.2 DSP Additions (HaasProcessor)

**1.2.1 Width Low Cut Filter**
- Add `BiquadFilter` highpass filter in the width processing path
- Apply before M/S width adjustment
- Only affects the side (S) channel to keep bass mono

**1.2.2 Output Gain Stage**
- Add gain multiplication after dry/wet mix
- Use smoothed parameter to avoid clicks

**1.2.3 Correction Speed Parameter**
- Expose attack/release time multiplier in `AutoPhaseCorrector`
- Map 0-100% to time range (e.g., 0% = 200ms, 100% = 10ms)

**1.2.4 Module Bypass Logic**
- Add bypass flags checked in `processBlock()`
- When bypassed, module passes signal unchanged

**Files to modify:**
- `Source/DSP/HaasProcessor.h` - Add filter, gain, bypass flags
- `Source/DSP/AutoPhaseCorrector.h` - Add speed parameter exposure

### 1.3 Color Constants

Create new color constants file or update existing:

```cpp
namespace ReOrderColors {
    // Backgrounds
    const Colour bgDarkest    { 0xff0d0d0d };
    const Colour bgDark       { 0xff1a1a1a };
    const Colour bgModule     { 0xff141414 };
    const Colour border       { 0xff2a2a2a };
    const Colour borderLight  { 0xff333333 };

    // Module accents
    const Colour accentDelay  { 0xffff66aa };  // Pink
    const Colour accentWidth  { 0xff00d4ff };  // Cyan
    const Colour accentPhase  { 0xffff9933 };  // Orange
    const Colour accentOutput { 0xff00ff88 };  // Green

    // Text
    const Colour textPrimary  { 0xffffffff };
    const Colour textSecondary{ 0xffcccccc };
    const Colour textMuted    { 0xff888888 };
    const Colour textDim      { 0xff666666 };
    const Colour textVeryDim  { 0xff444444 };

    // Status
    const Colour statusGood   { 0xff00ff88 };
    const Colour statusWarn   { 0xffffcc00 };
    const Colour statusError  { 0xffff3366 };
    const Colour correcting   { 0xffff6600 };
}
```

**Files to create/modify:**
- `Source/UI/ReOrderColors.h` (new file)

---

## Phase 2: LookAndFeel Overhaul

### 2.1 Create ReOrderLookAndFeel Class

Replace or extend `VoxProLookAndFeel` with new styling.

**2.1.1 Rotary Slider (`drawRotarySlider`)**

Per specification Section 6:
- 270° rotation range (-135° to +135°)
- Multi-layer glow effect on value arc
- 3D metallic knob body with radial gradient
- Center cap with subtle highlight
- Indicator dot on arc + line on knob
- Color retrieved from slider properties

**2.1.2 Linear Vertical Slider (`drawLinearSlider`)**

Per specification Section 7:
- Track: 12-14px wide, dark with inset shadow
- Fill: 6px centered, gradient with glow, from bottom
- Handle: 20×16px metallic gradient with grip lines
- Value display below

**2.1.3 Linear Horizontal Slider (`drawLinearSlider`)**

Per specification Section 8:
- Track: 8-10px tall
- Fill from left
- Smaller thumb
- Label row above (label left, value right)

**2.1.4 Power Button (`drawButtonBackground`)**

- 20×20px circle
- 2px border in accent color (on) or #444 (off)
- Fill: accent at 20% opacity (on) or transparent (off)

**2.1.5 Toggle Button (LINK, AUTO FIX)**

- Rectangular with rounded corners
- Off: dark bg, gray border/text
- On: gradient fill in accent, glow, dark or white text

**2.1.6 Bypass Button**

- Special red styling when active
- Gray when inactive (plugin processing)

**Files to create/modify:**
- `Source/UI/ReOrderLookAndFeel.h` (new)
- `Source/UI/ReOrderLookAndFeel.cpp` (new)
- Or heavily modify existing `VoxProLookAndFeel`

---

## Phase 3: New UI Components

### 3.1 Vertical Level Meter

**File:** `Source/UI/VerticalLevelMeter.h`

- 24 segments, 8×3px each, 2px gap
- Bottom-to-top fill
- Color zones: green (0-60%), yellow (60-75%), red (75-100%)
- Glow on active segments
- Accepts level value 0-100

### 3.2 Phase Correlation Meter (Enhanced)

**File:** `Source/UI/PhaseCorrelationMeter.h` (modify existing or new)

- 200px wide, 24px tall
- Gradient background overlay (red→orange→yellow→green)
- Movable threshold marker (vertical line)
- Position indicator (4×16px bar)
- Color changes based on correlation vs threshold
- Scale labels (-1, 0, +1) above
- Value readout below
- "CORRECTING" badge with pulse animation

### 3.3 Module Component (Base Class)

**File:** `Source/UI/ModuleComponent.h`

Base class for all 4 modules with:
- Standard header (power button, title, optional badge)
- Icon area
- Controls area
- Accent color property
- Active/inactive state handling
- Opacity reduction when inactive

### 3.4 Module Icons

**File:** `Source/UI/ModuleIcons.h`

SVG-style path drawing for:
- **HaasIcon**: Two offset sine waves
- **WidthIcon**: Expanding arrows from center
- **PhaseIcon**: Concentric circles with crossing waves
- **OutputIcon**: Speaker with sound waves

Each icon:
- 60×60px
- Stroke-based drawing
- Glow filter when active
- Optional animation state

### 3.5 Preset Selector Component

**File:** `Source/UI/PresetSelector.h`

- Left/right arrow buttons
- Central dropdown (ComboBox or custom)
- Preset name display
- Click to open full preset browser
- Save/Load/Rename functionality
- Integration with preset file system

### 3.6 Preset Manager

**File:** `Source/Presets/PresetManager.h`

- Scan preset directory on startup
- Load/save `.preset` files (XML or JSON format)
- Factory presets embedded in binary
- User preset directory (platform-specific)
- Preset data structure matching all parameters

---

## Phase 4: Main Editor Layout

### 4.1 Editor Restructure

**File:** `Source/PluginEditor.h` / `Source/PluginEditor.cpp`

Complete rewrite of layout:

```
┌─────────────────────────────────────────────────────────────┐
│  HEADER (48px)                                               │
│  [Logo] [PRO]        [← Preset ▼ →]              [BYPASS]   │
├────┬─────────┬─────────┬───────────┬─────────┬──────────────┤
│    │         │         │           │         │              │
│ IN │  DELAY  │  WIDTH  │ AUTO PHASE│ OUTPUT  │     OUT      │
│ 50 │  flex:1 │  flex:1 │  flex:1.3 │  flex:1 │      50      │
│    │         │         │           │         │              │
├────┴─────────┴─────────┴───────────┴─────────┴──────────────┤
│  FOOTER (40px)                                               │
│                    REORDER AUDIO                             │
└─────────────────────────────────────────────────────────────┘
```

**Window size:** 1000 × 625 px (fixed, no scaling for initial implementation)

### 4.2 Header Component

**File:** `Source/UI/HeaderComponent.h`

- Logo: Custom drawn (circle + crosshairs + "HAASFX" gradient text + "PRO" badge)
- Preset selector (center)
- Bypass button (right)

### 4.3 Footer Component

**File:** `Source/UI/FooterComponent.h`

- Centered "REORDER AUDIO" text
- "REORDER" in #444, "AUDIO" in cyan

### 4.4 Input/Output Meter Strips

**File:** `Source/UI/MeterStrip.h`

- 50px wide panel
- Vertical label (rotated)
- L/R VerticalLevelMeter pair
- dB readout at bottom

### 4.5 Delay Module Component

**File:** `Source/UI/DelayModule.h`

Layout:
```
[●] ──── DELAY ────
      [Haas Icon]
   [L Slider] [R Slider]
     0.0ms      15.0ms
     LEFT       RIGHT
       [LINK]
```

Components:
- Power button → `delayBypass`
- Left vertical slider → `delayLeft` (0-50ms)
- Right vertical slider → `delayRight` (0-50ms)
- Link button → `delayLink`
- Link logic: maintain offset when dragging

### 4.6 Width Module Component

**File:** `Source/UI/WidthModule.h`

Layout:
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

Components:
- Power button → `widthBypass`
- Width knob (90px) → `width` (0-200%)
- Low cut horizontal slider → `widthLowCut` (20-500Hz)
- Phase L button → `phaseLeft`
- Phase R button → `phaseRight`

### 4.7 Auto Phase Module Component

**File:** `Source/UI/PhaseModule.h`

Layout:
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

Components:
- Power button → `phaseBypass`
- "EXCLUSIVE" badge (static)
- Phase icon (animated when correcting)
- Correlation meter with threshold line
- AUTO FIX toggle → `autoPhase`
- Threshold knob (60px) → `phaseSafety` mapped to 0-1 (or new param)
- Speed knob (60px) → `correctionSpeed`
- "CORRECTING" badge (shown when active)

Special states:
- When correcting: orange border glow, tinted background

### 4.8 Output Module Component

**File:** `Source/UI/OutputModule.h`

Layout:
```
[●] ──── OUTPUT ────
      [Output Icon]

      [Gain Knob]
        +0.0dB
         GAIN

  DRY/WET          100%
  [━━━━━━━━━━━━━━━━━●]
```

Components:
- Power button → `outputBypass`
- Gain knob (80px) → `outputGain` (-12 to +12 dB)
- Dry/Wet horizontal slider → `mix` (0-100%)

---

## Phase 5: Preset System

### 5.1 Preset File Format

XML structure:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<HaasPreset version="1" name="Clean Stereo">
  <Parameters>
    <delayLeft>0.0</delayLeft>
    <delayRight>15.0</delayRight>
    <delayLink>false</delayLink>
    <width>150.0</width>
    <widthLowCut>250.0</widthLowCut>
    <phaseLeft>false</phaseLeft>
    <phaseRight>false</phaseRight>
    <autoPhase>true</autoPhase>
    <phaseSafety>0.3</phaseSafety>
    <correctionSpeed>50.0</correctionSpeed>
    <mix>100.0</mix>
    <outputGain>0.0</outputGain>
  </Parameters>
</HaasPreset>
```

### 5.2 Factory Presets

Embedded presets:
1. **Init** - All defaults
2. **Clean Stereo** - Subtle Haas, auto-phase on
3. **Wide Vocals** - 20ms R delay, 150% width
4. **Subtle Width** - 5ms delay, 120% width
5. **Mono Compatible** - Auto-phase strict, conservative settings
6. **Extreme Haas** - 40ms delay, 180% width, relaxed phase

### 5.3 Preset Directory

- **macOS:** `~/Library/Audio/Presets/ReOrder Audio/Haas FX Pro/`
- **Windows:** `%APPDATA%/ReOrder Audio/Haas FX Pro/Presets/`

### 5.4 Preset Browser UI

- Dropdown shows: [Factory] section + [User] section
- Right-click or menu button for Save/Save As/Rename/Delete
- Modified indicator (*) when preset has unsaved changes

---

## Phase 6: Animation & Polish

### 6.1 Module Icon Animations

Using `juce::Timer` or `juce::AnimatedPosition`:

- **Haas Icon**: Gentle wave motion (translateY oscillation)
- **Width Icon**: Expand/contract arrows
- **Phase Icon**: Wave compression animation when correcting
- **Output Icon**: Sound wave fade-out animation

### 6.2 Correction State Animation

- Smooth transition of module border color
- Background tint fade in/out
- "CORRECTING" badge pulse (opacity 1↔0.5, 1s cycle)

### 6.3 Control Feedback

- Hover states: Accent color border on handles
- Active drag: Slight glow increase
- Value changes: Smooth visual transitions

### 6.4 Meter Smoothing

- Level meters: ~50ms visual decay
- Correlation meter: ~100ms position smoothing
- Peak hold: 1-2 second hold before decay

---

## Implementation Order

### Sprint 1: Foundation (Files: 8, Estimated complexity: Medium)
1. Add new parameters to PluginProcessor
2. Create ReOrderColors.h
3. Implement DSP additions (low cut, output gain, speed param, bypasses)
4. Basic ReOrderLookAndFeel skeleton

### Sprint 2: Core Components (Files: 6, Estimated complexity: High)
1. ReOrderLookAndFeel - rotary slider
2. ReOrderLookAndFeel - linear sliders (both orientations)
3. ReOrderLookAndFeel - buttons (power, toggle, bypass)
4. VerticalLevelMeter component
5. Enhanced PhaseCorrelationMeter
6. ModuleIcons drawing

### Sprint 3: Module Components (Files: 5, Estimated complexity: High)
1. ModuleComponent base class
2. DelayModule with link logic
3. WidthModule with phase buttons
4. PhaseModule with correlation meter
5. OutputModule

### Sprint 4: Layout & Structure (Files: 4, Estimated complexity: Medium)
1. HeaderComponent with logo
2. FooterComponent
3. MeterStrip component
4. Main PluginEditor layout restructure

### Sprint 5: Preset System (Files: 3, Estimated complexity: Medium)
1. PresetManager class
2. PresetSelector UI component
3. Factory preset definitions
4. File I/O and directory handling

### Sprint 6: Polish (Files: 0, Estimated complexity: Low-Medium)
1. Icon animations
2. State transition animations
3. Hover/active states
4. Meter smoothing
5. Testing and bug fixes

---

## File Summary

### New Files to Create (17 files)
```
Source/
├── UI/
│   ├── ReOrderColors.h
│   ├── ReOrderLookAndFeel.h
│   ├── ReOrderLookAndFeel.cpp
│   ├── VerticalLevelMeter.h
│   ├── ModuleIcons.h
│   ├── ModuleComponent.h
│   ├── DelayModule.h
│   ├── WidthModule.h
│   ├── PhaseModule.h
│   ├── OutputModule.h
│   ├── HeaderComponent.h
│   ├── FooterComponent.h
│   ├── MeterStrip.h
│   └── PresetSelector.h
└── Presets/
    ├── PresetManager.h
    ├── PresetManager.cpp
    └── FactoryPresets.h
```

### Files to Modify (6 files)
```
Source/
├── PluginProcessor.h      (add parameters)
├── PluginProcessor.cpp    (add parameters, connect DSP)
├── PluginEditor.h         (complete rewrite of layout)
├── PluginEditor.cpp       (complete rewrite of layout)
├── DSP/
│   ├── HaasProcessor.h    (add low cut, gain, bypasses)
│   └── AutoPhaseCorrector.h (expose speed parameter)
```

### Files to Potentially Remove/Archive
```
Source/UI/
├── VoxProLookAndFeel.h    (replace with ReOrder)
├── VoxProLookAndFeel.cpp  (replace with ReOrder)
```

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Complex LookAndFeel drawing | High | Medium | Iterate on knob/slider drawing, test frequently |
| Preset file permissions | Medium | Low | Use JUCE's recommended directories |
| Module bypass causing audio glitches | Medium | High | Use smoothed bypass with crossfade |
| Layout doesn't fit 1000×625 | Low | Medium | Build layout incrementally, test sizing |
| Animation performance | Low | Low | Use efficient timer callbacks, profile |

---

## Testing Checklist

### Visual
- [ ] All module colors match specification
- [ ] Knob arcs glow with correct colors
- [ ] Vertical slider fills from bottom
- [ ] Horizontal slider fills from left
- [ ] Handles have 3D metallic appearance
- [ ] Module headers show power buttons correctly
- [ ] Icons display and animate properly
- [ ] Correlation meter shows threshold line
- [ ] "CORRECTING" badge pulses when active

### Functional
- [ ] All parameters save/restore with plugin state
- [ ] Delay LINK maintains offset correctly
- [ ] Module power buttons bypass processing
- [ ] Auto phase threshold knob affects correction trigger
- [ ] Speed knob affects correction timing
- [ ] Low cut keeps bass mono
- [ ] Output gain provides ±12dB range
- [ ] Presets save and load correctly
- [ ] Factory presets load properly

### Audio
- [ ] No clicks/pops on parameter changes
- [ ] No audio glitches on module bypass
- [ ] Metering accurately reflects signal levels
- [ ] Correlation meter matches actual phase relationship

---

## Success Criteria

1. UI matches mockup specifications within reasonable tolerance
2. All existing DSP functionality preserved
3. New parameters (link, low cut, speed, gain) work correctly
4. Preset system saves/loads user and factory presets
5. No regressions in audio quality or stability
6. Plugin validates in major DAWs (Logic, Ableton, Pro Tools)

---

*Plan Version: 1.0*
*Created: December 2024*
*Target: HAAS FX Pro v2.0*
