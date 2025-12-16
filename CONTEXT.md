# Haas Align Delay Plugin - Development Context

## Overview
A JUCE-based VST3/AU audio plugin combining Haas effect stereo widening with track alignment delay functionality. Features a modern XVOX Pro-inspired UI design.

## Plugin Features

### Audio Processing
- **Delay Left/Right**: Independent delay controls for each channel (0-50ms range)
- **Width**: Stereo width control (Haas effect)
- **Mix**: Dry/wet mix control
- **Phase Invert**: Independent phase inversion for L/R channels
- **Auto Phase**: Automatic phase correlation detection and correction
- **Bypass**: Full bypass functionality

### Metering
- Input level meter (segmented, color-coded)
- Output level meter (segmented, color-coded)
- Stereo correlation meter (-1 to +1)

## UI Design - XVOX Pro Style

### Color Palette
```cpp
accentPurple = 0xff7B5FFF   // Main purple accent
accentPink = 0xffff6b9d     // Pink accent
accentCyan = 0xff00d4ff     // Cyan for meters
accentYellow = 0xffffd93d   // Yellow for Auto Phase
textWhite = 0xffe8e8f0      // Primary text
textGray = 0xff8888a0       // Secondary text
textDark = 0xff4a4a5a       // Tertiary text
meterGreen = 0xff00ff88     // Meter green
meterYellow = 0xffffd700    // Meter warning
meterRed = 0xffff4444       // Meter clip
bgDark = 0xff0a0a0e         // Background
```

### Knob Design (XVOX Pro Style)
1. **Glowing Purple Tick Marks** around perimeter
   - 13 tick marks (like clock face)
   - Multi-layer glow effect when lit:
     - Outer bloom (alpha 0.15, width 4x)
     - Middle glow (alpha 0.3, width 2.5x)
     - Inner glow (alpha 0.5, width 1.5x)
     - Core bright tick (#9b7fff)
   - Dark gray when unlit (#2a2a32)
   - Lights up progressively as value increases

2. **Multi-Layer Drop Shadow** (lifted/embossed effect)
   - 4 shadow layers extending 10px, 7px, 5px, 3px outward
   - Opacity: 8%, 12%, 18%, 30%
   - Downward offset for 3D depth

3. **Knob Body**
   - Dark beveled outer ring (#1a1a20)
   - Matte dark gray body with subtle gradient (#2e2e36 to #222228)
   - Inner recessed circle for rubber look (#1c1c22)
   - White indicator line (thin, near edge)

### Layout
- **Header**: Logo + BYPASS button
- **Main Content**: 4 panels (DELAY L, DELAY R, WIDTH, MIX)
- **Footer**: INPUT meter, Correlation meter, OUTPUT meter
- **Connection Bar**: Between DELAY L and DELAY R panels with phase buttons

### Buttons
- Phase L/R buttons (purple when active)
- Auto Phase button (yellow when active)
- Bypass button (subtle dark style)

## File Structure

```
HaasAlignDelay/
├── Source/
│   ├── PluginProcessor.h/cpp    # Audio processing
│   ├── PluginEditor.h/cpp       # UI implementation
│   └── BinaryData/              # Logo and assets
├── build/                       # CMake build directory
├── CMakeLists.txt              # Build configuration
├── CLAUDE.md                   # Development rules
└── CONTEXT.md                  # This file
```

## Key Code Sections

### PluginEditor.h
- `VoxProLookAndFeel` class - Custom look and feel
- `HaasAlignDelayEditor` class - Main editor component
- Scale presets: 50%, 75%, 100%, 150%
- Base dimensions: 580x380

### PluginEditor.cpp

#### `drawRotarySlider()` (lines ~15-145)
- Glowing tick marks implementation
- Multi-layer shadow effect
- Knob body with gradients
- White indicator line

#### `drawButtonBackground()` (lines ~125-205)
- XVOX-style button rendering
- Phase buttons (purple glow when on)
- Auto Phase (yellow glow when on)
- Bypass button (subtle dark)

#### `drawMeter()` (lines ~355-402)
- Segmented meter (40 segments)
- Color gradient: green → cyan → yellow → orange → red

#### `drawCorrelationMeter()` (lines ~404-446)
- Center-referenced meter
- Color changes based on correlation value

#### `paint()` (lines ~448-642)
- Header with logo
- Panel drawing
- Knob labels and values
- Footer with meters

#### `resized()` (lines ~644-705)
- Component positioning
- Scale-based sizing

## Recent Changes

### XVOX Pro Knob Style
- Changed from simple gradient knobs to chunky matte style
- Added glowing purple tick marks around perimeter
- Implemented multi-layer soft drop shadow for 3D effect

### Scaling System (In Progress)
- Moving from free-form resizing to fixed scale presets
- Planned scales: 50%, 75%, 100%, 150%
- Scale picker button (like FabFilter Pro-Q)

## Build Commands

```bash
# Configure (first time)
cd /Users/reorderdj/Projects/HaasAlignDelay
mkdir build && cd build
cmake ..

# Build
cd /Users/reorderdj/Projects/HaasAlignDelay/build
cmake --build . --config Release -j8

# Open standalone
open "/Users/reorderdj/Projects/HaasAlignDelay/build/HaasAlignDelay_artefacts/Release/Standalone/Haas Align Delay.app"
```

## Plugin Installation Paths
- AU: `~/Library/Audio/Plug-Ins/Components/Haas Align Delay.component`
- VST3: `~/Library/Audio/Plug-Ins/VST3/Haas Align Delay.vst3`

## Completed Features
- [x] XVOX Pro style knobs with glowing purple tick marks
- [x] Multi-layer soft drop shadow for 3D embossed look
- [x] Fixed scale presets (50%, 75%, 100%, 150%)
- [x] Scale picker button in bottom-right corner (like FabFilter Pro-Q)
- [x] Removed free-form resizing - only preset scales

## Current State
The PluginEditor.h and PluginEditor.cpp files are in Source/ with:
- Fixed scale presets: 50%, 75%, 100%, 150% (base: 580x380)
- Scale button cycles through presets on click
- No resizable corner component - window is fixed size per preset
- All UI elements scale proportionally with the selected preset

## Development Notes
- Uses JUCE framework
- Builds VST3, AU, and Standalone targets
- Logo loaded from BinaryData
- 30Hz timer for meter updates
