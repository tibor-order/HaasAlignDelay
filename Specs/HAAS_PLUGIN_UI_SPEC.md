# HAAS FX Pro - JUCE Plugin UI Design Specification

## Overview

This document provides complete specifications for implementing the HAAS FX Pro audio plugin UI in JUCE C++. The design is a professional, dark-themed interface with neon accent colors and 3D metallic controls.

---

## 1. Global Dimensions & Layout

### Plugin Window
- **Size:** 1000 x 625 pixels
- **Background:** Linear gradient `#1a1a1a` → `#0d0d0d` (top to bottom)
- **Border:** 1px solid `#333333`
- **Corner Radius:** 12px
- **Box Shadow:** `0 20px 60px rgba(0,0,0,0.5)`, inner highlight `0 1px 0 rgba(255,255,255,0.05)`

### Main Sections
```
┌─────────────────────────────────────────────────────────────┐
│  HEADER (48px height)                                        │
├────┬─────────┬─────────┬───────────┬─────────┬────┤
│    │         │         │           │         │    │
│ IN │  DELAY  │  WIDTH  │ AUTO PHASE│ OUTPUT  │OUT │
│    │  flex:1 │  flex:1 │  flex:1.3 │  flex:1 │    │
│50px│         │         │           │         │50px│
│    │         │         │           │         │    │
├────┴─────────┴─────────┴───────────┴─────────┴────┤
│  FOOTER (40px height)                                        │
└─────────────────────────────────────────────────────────────┘
```

### Spacing
- **Outer padding:** 15px
- **Module gap:** 15px
- **Internal module padding:** 15px

---

## 2. Color Palette

### Base Colors
| Name | Hex | Usage |
|------|-----|-------|
| Background Dark | `#0d0d0d` | Main background bottom |
| Background Light | `#1a1a1a` | Main background top, inputs |
| Panel Background | `#141414` | Module backgrounds |
| Border Dark | `#2a2a2a` | Module borders, dividers |
| Border Medium | `#333333` | Control borders |
| Border Light | `#444444` | Highlights, hover states |

### Accent Colors (Per Module)
| Module | Primary Color | Hex | Alpha Variants |
|--------|--------------|-----|----------------|
| **DELAY** | Pink | `#ff66aa` | 33% for backgrounds, 66% for glows |
| **WIDTH** | Cyan | `#00d4ff` | 22% for subtle, 44% for medium |
| **AUTO PHASE** | Orange | `#ff9933` | Correcting state: `#ff6600` |
| **OUTPUT** | Green | `#00ff88` | For active states and values |

### Text Colors
| Usage | Hex |
|-------|-----|
| Primary text | `#ffffff` |
| Secondary text | `#cccccc` |
| Muted text | `#888888` |
| Disabled text | `#666666` |
| Very muted | `#444444` |

### Status Colors
| Status | Hex |
|--------|-----|
| Good/Positive | `#00ff88` |
| Warning | `#ffcc00` |
| Error/Negative | `#ff3366` |
| Bypass Active | `#ff3366` |

---

## 3. Typography

### Font Families
- **Primary UI:** Inter (or system sans-serif fallback)
- **Monospace/Values:** JetBrains Mono (or system monospace)

### Font Sizes
| Element | Size | Weight | Style |
|---------|------|--------|-------|
| Logo "HAAS" | 18px | 600 (Bold) | Letter-spacing: 2px |
| Module titles | 13px | 600 (Bold) | Letter-spacing: 2px, UPPERCASE |
| Control labels | 11px | 400 | Letter-spacing: 1px, UPPERCASE |
| Value displays | 11-12px | 400 | Monospace |
| Small labels | 9-10px | 400 | Letter-spacing: 0.5-1px |
| Badges | 10px | 600 | - |

---

## 4. Header Component (48px)

### Layout
```
[Logo] [PRO Badge]     [< Preset Selector >]     [BYPASS Button]
```

### Logo
- **Icon:** 28x28px circle with crosshairs
  - Outer circle: 2px stroke `#00d4ff`
  - Center dot: 6px filled `#00d4ff`
  - Crosshairs: 2px stroke `#00d4ff`
- **Text:** "HAAS" with gradient `#00d4ff` → `#00ff88`
- **"FX":** Same gradient at 60% opacity

### PRO Badge
- **Size:** ~32x18px
- **Background:** `#00d4ff` at 15% opacity
- **Text:** "PRO" in `#00d4ff`, 10px bold
- **Border-radius:** 3px

### Preset Selector
- **Container:** Centered, min-width 180px
- **Background:** `#0d0d0d`
- **Border:** 1px solid `#333333`
- **Border-radius:** 6px
- **Padding:** 8px 24px
- **Arrows:** "←" / "→" in `#666666`, 4px padding

### Bypass Button
- **Size:** ~70x28px
- **Inactive:** Background `#2a2a2a`, border `#444444`, text `#888888`
- **Active:** Background `#ff3366`, border `#ff3366`, text `#ffffff`
- **Border-radius:** 6px
- **Font:** 12px, 600 weight, letter-spacing 1px

---

## 5. Module Structure

Each module follows this structure:

```
┌──────────────────────────────┐
│ [●] MODULE TITLE    [Badge?] │  ← Header (40px)
├──────────────────────────────┤
│                              │
│         [ICON 60x60]         │  ← Icon area (~70px)
│                              │
├──────────────────────────────┤
│                              │
│         [CONTROLS]           │  ← Controls area (flex)
│                              │
└──────────────────────────────┘
```

### Module Background
- **Color:** `#141414`
- **Border:** 1px solid `#2a2a2a`
- **Border-radius:** 10px
- **Overflow:** hidden

### Module Header
- **Height:** ~40px
- **Padding:** 10px 15px
- **Border-bottom:** 1px solid `#2a2a2a`
- **Layout:** Power button (left), Title (center), Badge/spacer (right)

### Power Button
- **Size:** 20x20px
- **Shape:** Circle with 2px border
- **Inactive:** Border `#444444`, transparent fill
- **Active:** Border = module accent color, fill = accent at 20% opacity

---

## 6. Rotary Knob Component

### Dimensions & Proportions
For a knob of `size` pixels:
- **Outer arc radius:** `size * 0.88 / 2`
- **Knob body radius:** `size * 0.72 / 2`
- **Center cap radius:** `bodyRadius * 0.35`
- **Arc stroke width:** `3.5 * (size / 84)`

### Rotation Range
- **Start angle:** -135° (225° or `π * 1.25` radians)
- **End angle:** +135° (495° or `π * 2.75` radians)
- **Total sweep:** 270°

### Visual Layers (bottom to top)

1. **Drop Shadow**
   - Color: Black at decreasing opacity (6-8%)
   - Offset: Down and slightly right
   - Multiple layers for soft effect

2. **Background Arc Track**
   - Color: `#222222`
   - Stroke width: arc width + 2px
   - Full 270° sweep

3. **Value Arc (Glowing)**
   - Color: Module accent color
   - From start angle to current value angle
   - Glow layers:
     - Outer: accent at 8% opacity, width + 20px
     - Medium: accent at 15% opacity, width + 12px
     - Inner: accent at 30% opacity, width + 6px
     - Core: accent at 100%, normal width
     - Highlight: accent brighter 30%, half width

4. **Knob Base Ring**
   - Color: `#1a1a1a`
   - Full circle at knob radius

5. **Knob Body**
   - Gradient: `#3a3a3a` (top-left) → `#1a1a1a` (bottom-right)
   - Radial gradient for 3D effect
   - Border: 2px `#333333`

6. **Top Highlight Arc**
   - Color: White at 10% opacity
   - Partial arc on top-left quadrant

7. **Center Cap**
   - Gradient: `#444444` (top) → `#222222` (bottom)
   - Inset shadow: `0 1px 2px rgba(255,255,255,0.1)`

8. **Indicator Dot on Arc**
   - Position: On arc at current value angle
   - Glow: accent at 50% opacity, 2.5x dot radius
   - Dot: White, 4px radius (scaled)

9. **Indicator Line on Knob**
   - From inner radius (cap edge) to outer radius (body edge - 4px)
   - Glow: accent at 40%, 5px width
   - Line: White at 95%, 2px width

### Value Display Box (below knob)
- **Background:** `#1a1a1a`
- **Border:** 1px solid `#333333`
- **Border-radius:** 4px
- **Padding:** 4px 12px
- **Min-width:** 60px
- **Text:** Monospace, 11px, module accent color
- **Format:** Value with 1 decimal + unit (e.g., "100%", "-1.6dB")

### Label (below value box)
- **Font:** 11px, uppercase, letter-spacing 1px
- **Color:** `#888888`

---

## 7. Vertical Slider Component

### Dimensions
- **Track width:** 12-14px
- **Track height:** 100-120px (configurable)
- **Handle:** 20px wide × 16px tall

### Visual Layers

1. **Track Background**
   - Color: `#1a1a1a`
   - Border: 1px solid `#333333`
   - Border-radius: 6-7px
   - Inner shadow: `inset 0 2px 4px rgba(0,0,0,0.5)`

2. **Inner Track (darker)**
   - Color: `#0a0a0a`
   - Inset 2px from outer track
   - Border-radius: 5px

3. **Fill (from bottom)**
   - Width: 6px, centered
   - Gradient: accent color (top) → accent at 50% (bottom)
   - Border-radius: 3px
   - Glow: accent at 30-40%, expanded 3px horizontally

4. **Handle/Thumb**
   - Size: 20px × 16px (wider than track)
   - Position: Centered on track at value position
   - Gradient: `#555555` (top) → `#2a2a2a` (bottom)
   - Border: 1px solid `#444444` (hover: accent color)
   - Border-radius: 3-4px
   - Shadow: Black at 30-40%, offset 1-2px down
   - Top highlight: White at 15%, 1px line near top
   - Grip lines: 3 horizontal lines in `#1a1a1a`, spaced 3px

### Value Display (below slider)
- **Font:** Monospace, 11-12px, bold
- **Color:** Module accent color
- **Format:** Value + "ms" (e.g., "0.1ms")

### Label (below value)
- **Font:** 10px, uppercase
- **Color:** `#666666`

---

## 8. Horizontal Slider Component

### Dimensions
- **Track height:** 8-10px
- **Track width:** Full container width - padding
- **Thumb:** 12-14px wide × track height + 6px

### Visual Layers

1. **Track Background**
   - Color: `#1a1a1a`
   - Border: 1px solid `#333333`
   - Border-radius: 4-5px

2. **Inner Track**
   - Color: `#0a0a0a`
   - Reduced by 2px
   - Border-radius: 3px

3. **Fill (from left)**
   - Height: track height - 4px
   - Color: Module accent color
   - Glow: accent at 40%, expanded vertically
   - Border-radius: 3px

4. **Thumb**
   - Gradient: `#555555` → `#2a2a2a`
   - Border: 1px solid `#444444` (hover: accent)
   - Border-radius: 3-4px
   - Shadow: subtle, down

### Label Row (above slider)
- **Layout:** Label on left, value on right
- **Label:** 10px, `#666666`, uppercase
- **Value:** 10px, module accent color, e.g., "20Hz" or "100%"

---

## 9. DELAY Module Specifics

### Color: `#ff66aa` (Pink)

### Icon (60x60px)
Two offset sine waves representing Haas delay:
- Upper wave: `M5 30 Q15 15, 25 30 T45 30 T55 30`
  - Stroke: 2px, color at 50% opacity
- Lower wave: `M10 35 Q20 20, 30 35 T50 35`
  - Stroke: 2.5px, full color
- When active: `drop-shadow(0 0 10px color)`, wave animation

### Controls Layout
```
┌────────────────────────────┐
│  [Left Slider] [Right Slider] │
│                               │
│      0.0ms         0.0ms      │
│       LEFT         RIGHT      │
│                               │
│         [LINK Button]         │
└────────────────────────────┘
```

### Control Specs
- **Left/Right Sliders:** LinearVertical, 0-50ms range, height ~100px
- **Link Button:**
  - Size: ~60x24px
  - Inactive: Background `#1a1a1a`, border `#333333`, text `#666666`
  - Active: Background `#ff66aa33`, border `#ff66aa`, text `#ff66aa`
  - Font: 10px, letter-spacing 1px

---

## 10. WIDTH Module Specifics

### Color: `#00d4ff` (Cyan)

### Icon (60x60px)
Expanding arrows from center:
- Center dot: 4px radius, filled
- 4 arrows pointing outward (top-left, top-right, bottom-left, bottom-right)
- Arrow paths with arrowheads
- When active: Expand animation, glow

### Controls Layout
```
┌────────────────────────────┐
│                            │
│        [Width Knob]        │
│          100%              │
│       STEREO WIDTH         │
│                            │
│  LOW CUT          20Hz     │
│  [━━━━━●━━━━━━━━━━━━━━━]  │
│                            │
└────────────────────────────┘
```

### Control Specs
- **Width Knob:** 90x90px, range 100-200%
- **Low Cut Slider:** LinearHorizontal, range 20-500Hz

---

## 11. AUTO PHASE Module Specifics

### Color: `#ff9933` (Orange)
### Correcting Color: `#ff6600`

### Header Badge
- "EXCLUSIVE" badge on right
- Background: `#ff993322`
- Text: `#ff9933`, 9px

### Correcting State
When correlation < threshold:
- Module background: `#1a1410`
- Border: `#ff660066`
- Box-shadow: `0 0 30px #ff660022`

### Icon (60x60px)
Phase correlation symbol:
- Outer circle: 20px radius, 1.5px stroke, 30% opacity
- Inner circle: 12px radius, 1.5px stroke, 50% opacity
- Two sine waves (one inverted):
  - `M15 30 Q22 20, 30 30 T45 30`
  - `M15 30 Q22 40, 30 30 T45 30`
- Correcting state: Orange color, phaseCorrect animation
- Shows "AUTO" text when correcting

### Controls Layout
```
┌────────────────────────────────┐
│                                │
│          [Phase Icon]          │
│                                │
│   -1        0        +1        │
│  [━━━━━━━━━|━━━━━━●━━━━━━━━]  │
│           +1.00                │
│                                │
│      [AUTO FIX ON/OFF]         │
│                                │
│  [Threshold]    [Speed]        │
│     0.27          50%          │
│   THRESHOLD      SPEED         │
│                                │
└────────────────────────────────┘
```

### Correlation Meter
- **Width:** ~200px
- **Height:** 24px
- **Background:** `#0a0a0a`
- **Gradient overlay:** `#ff3366` → `#ff6633` → `#ffcc00` → `#66ff66` → `#00ff88` at 30% opacity
- **Threshold marker:** 2px vertical line
  - Inactive: `#444444`
  - Active: `#ff6600` with glow
- **Position indicator:** 4px × 16px vertical bar
  - Color: `#ff3366` if below threshold, `#00ff88` if above
  - Glow: 12px
- **Scale labels:** -1, 0, +1 above meter, 9px monospace, `#666666`
- **Value display:** Below meter, 12px monospace, colored by status

### Auto Fix Button
- **Size:** ~110x32px (or padding 10px 24px)
- **Inactive:** Background `#1a1a1a`, border `#333333`, text `#666666`
- **Active:** Gradient `#ff9933` → `#ff6600`, border `#ff9933`, text `#000000`
- **Active glow:** `0 0 20px #ff993366`
- **Font:** 11px, 600 weight, letter-spacing 1px
- **Text:** "AUTO FIX ON" or "AUTO FIX OFF"

### Control Specs
- **Threshold Knob:** 60x60px, range 0-1, shows 2 decimal places
- **Speed Knob:** 60x60px, range 0-100%, shows integer + "%"

---

## 12. OUTPUT Module Specifics

### Color: `#00ff88` (Green)

### Icon (60x60px)
Speaker icon with sound waves:
- Speaker body: Rounded rect 10x16px at x=18
- Speaker cone: Trapezoid path
- Sound waves: Two curved arcs
  - Inner: `M42 20 Q50 30, 42 40`
  - Outer: `M46 15 Q58 30, 46 45`
- When active: soundWave animation, full opacity
- Inactive: 30% / 20% opacity

### Controls Layout
```
┌────────────────────────────┐
│                            │
│        [Gain Knob]         │
│          -1.6dB            │
│           GAIN             │
│                            │
│  DRY/WET           100%    │
│  [━━━━━━━━━━━━━━━━━━━●]   │
│                            │
└────────────────────────────┘
```

### Control Specs
- **Gain Knob:** 80x80px, range -12dB to +12dB
  - Format: Sign + value + "dB" (e.g., "+0.0dB", "-1.6dB")
- **Dry/Wet Slider:** LinearHorizontal, range 0-100%

---

## 13. Input/Output Meters

### Container
- **Width:** 50px
- **Background:** `#0d0d0d`
- **Border:** 1px solid `#2a2a2a`
- **Border-radius:** 8px
- **Padding:** 10px 8px

### Layout
```
┌────────┐
│ I      │  ← Vertical label
│ N      │
│ P      │
│ U      │
│ T      │
│        │
│ [L][R] │  ← Level meters
│        │
│ 0.0dB  │  ← Peak value
└────────┘
```

### Level Meter Bar
- **Segments:** 24 horizontal bars
- **Bar size:** 8px wide × 3px tall
- **Bar gap:** 2px
- **Total height:** ~120px
- **Direction:** Bottom to top (column-reverse)

### Segment Colors
| Segment Range | Color |
|---------------|-------|
| 0-59% (0-14) | `#00ff88` |
| 60-74% (15-17) | `#ffcc00` |
| 75-100% (18-23) | `#ff3366` |

- **Active:** Full color with glow `0 0 4px color66`
- **Inactive:** `#222222`

### Labels
- **Vertical label:** 9px, `#666666`, letter-spacing 1px, rotated 180°
- **Channel labels:** 9px monospace, `#666666` (L, R)
- **Peak value:** 9px monospace, `#00ff88`

---

## 14. Footer Component (40px)

### Layout
- Centered content
- Background: `#0d0d0d`
- Border-top: 1px solid `#2a2a2a`

### Content
```
REORDER AUDIO
```
- "REORDER" in `#444444`
- "AUDIO" in `#00d4ff`
- Font: 11px, letter-spacing 2px

---

## 15. Animations (Reference Only)

CSS keyframes for reference (implement equivalent in JUCE if needed):

```css
@keyframes wave {
  0%, 100% { transform: translateY(0); }
  50% { transform: translateY(-3px); }
}

@keyframes expandLeft {
  0%, 100% { transform: translateX(0); }
  50% { transform: translateX(-3px); }
}

@keyframes expandRight {
  0%, 100% { transform: translateX(0); }
  50% { transform: translateX(3px); }
}

@keyframes phaseCorrect {
  0%, 100% { transform: scaleY(1); }
  50% { transform: scaleY(0.8); }
}

@keyframes soundWave {
  0% { opacity: 1; transform: translateX(0); }
  100% { opacity: 0; transform: translateX(5px); }
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}
```

---

## 16. JUCE Implementation Notes

### LookAndFeel
Create a custom `LookAndFeel_V4` subclass implementing:
- `drawRotarySlider()` - Follow Section 6
- `drawLinearSlider()` - Follow Sections 7 & 8
- `drawButtonBackground()` - For power buttons, link, bypass, auto fix
- `drawToggleButton()` - If using toggle buttons

### Color Storage
Store accent colors in slider properties for reliable retrieval:
```cpp
slider.getProperties().set("accentColor", (juce::int64)color.getARGB());
```

Retrieve in LookAndFeel:
```cpp
auto colorProp = slider.getProperties()["accentColor"];
if (!colorProp.isVoid())
    accentColour = juce::Colour((juce::uint32)(juce::int64)colorProp);
```

### Slider Styles
- Rotary knobs: `Slider::RotaryVerticalDrag`
- Vertical sliders: `Slider::LinearVertical`
- Horizontal sliders: `Slider::LinearHorizontal`
- All: `Slider::NoTextBox` (draw values manually)

### Drawing Order
For modules using `paint()`:
1. Draw module backgrounds, borders, headers
2. Draw icons
3. Draw static elements (labels, meters)
4. Let JUCE components (sliders) render on top via LookAndFeel

### Positioning
In `resized()`:
1. Calculate module bounds
2. Position interactive components (sliders, buttons) precisely
3. Store bounds for paint() reference if needed

### Glow Effects
Use multiple overlapping fills/strokes with decreasing opacity:
```cpp
// Outer glow
g.setColour(color.withAlpha(0.1f));
g.fillEllipse(bounds.expanded(10));
// Medium glow
g.setColour(color.withAlpha(0.2f));
g.fillEllipse(bounds.expanded(5));
// Core
g.setColour(color);
g.fillEllipse(bounds);
```

### Gradients
Use `juce::ColourGradient` for 3D effects:
```cpp
juce::ColourGradient gradient(
    juce::Colour(0xff3a3a3a), x1, y1,  // Light (top-left)
    juce::Colour(0xff1a1a1a), x2, y2,  // Dark (bottom-right)
    true);  // Radial
g.setGradientFill(gradient);
```

---

## 17. Parameter Mapping

| Parameter | ID | Range | Default | Display |
|-----------|-----|-------|---------|---------|
| Delay Left | `delayLeft` | 0-50 ms | 0 | X.Xms |
| Delay Right | `delayRight` | 0-50 ms | 15 | X.Xms |
| Stereo Width | `width` | 100-200% | 150 | XXX% |
| Low Cut | `lowCut` | 20-500 Hz | 250 | XXXHz |
| Threshold | `threshold` | 0-1 | 0.3 | X.XX |
| Speed | `speed` | 0-100% | 50 | XX% |
| Output Gain | `outputGain` | -12 to +12 dB | 0 | ±X.XdB |
| Dry/Wet | `dryWet` | 0-100% | 100 | XXX% |
| Bypass | `bypass` | bool | false | - |
| Delay Link | `delayLink` | bool | false | - |
| Auto Phase | `autoPhase` | bool | true | - |

---

## 18. Testing Checklist

### Visual Verification
- [ ] All module colors match specification
- [ ] Knob arcs glow properly with correct colors
- [ ] Vertical slider fills show from bottom
- [ ] Horizontal slider fills show from left
- [ ] Handles have 3D metallic gradient
- [ ] Value displays show correct formatting
- [ ] Module headers have power buttons with correct states
- [ ] Icons display with proper glow when active

### Interaction Testing
- [ ] All rotary knobs respond to vertical drag
- [ ] All sliders respond to drag
- [ ] Power buttons toggle module on/off states
- [ ] Link button toggles linked behavior
- [ ] Bypass button toggles bypass state
- [ ] Auto Fix button toggles auto phase correction
- [ ] Hover states show accent color borders

### Value Verification
- [ ] Delay values display as "X.Xms"
- [ ] Width displays as "XXX%"
- [ ] Low cut displays as "XXHz"
- [ ] Threshold displays as "X.XX"
- [ ] Speed displays as "XX%"
- [ ] Gain displays as "±X.XdB"
- [ ] Dry/Wet displays as "XXX%"

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | Dec 2024 | Initial design specification |
| 1.1 | Dec 2024 | MeterStrip: Horizontal labels (not rotated), 48 high-res segments, L/R labels, green dB readout |
| 1.2 | Dec 2024 | Haas icon: 50% size, two overlapping waves with animated delayed wave (75% opacity, vertical oscillation) |

---

*End of Specification*
