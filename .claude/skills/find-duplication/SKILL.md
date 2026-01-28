---
name: find-duplication
description: Identify duplicated code across ESP32 files that could be refactored into shared modules
allowed-tools: Read, Grep, Glob
---

# Find Code Duplication

Identify duplicated code across files that could be refactored into shared modules.

## Known Duplication Areas

### 1. Battery Monitoring
Both TX and RX have identical:
- `readBatteryVoltage()` function
- Battery voltage to percentage conversion
- Low battery detection logic
- Battery icon drawing

### 2. Joystick Visualization
- `drawJoystick()` function (circles, dots, crosshairs)
- Direction calculation logic
- Deadzone handling

### 3. Display Utilities
- Screen clear and refresh pattern
- Text positioning helpers
- Bar graph drawing

### 4. Motor Speed Calculation
- map() from joystick to motor speed
- Deadzone application
- Direction determination

### 5. Communication Setup
- BLE initialization boilerplate
- WiFi connection sequence
- mDNS setup

## Analysis Process

1. Read all .ino files
2. Identify functions with >80% similarity
3. Find repeated code blocks (>5 lines)
4. Calculate total duplicated lines

## Output Format

```
=== DUPLICATION REPORT ===

IDENTICAL FUNCTIONS:
- readBatteryVoltage() in TX and RX (15 lines each)
- drawJoystick() in TX and RX (25 lines each)

SIMILAR CODE BLOCKS:
- Battery icon drawing: TX:L100-120, RX:L200-220 (90% similar)
- BLE setup: TX:L50-80, RX:L60-90 (85% similar)

TOTAL DUPLICATION:
- Lines duplicated: ~150
- Files affected: 3
- Potential reduction: ~100 lines

REFACTORING SUGGESTIONS:
1. Create shared/battery.h for battery functions
2. Create shared/display.h for common drawing
3. Create shared/joystick.h for input processing

RECOMMENDED SHARED HEADER STRUCTURE:
shared/
  common_types.h    - JoystickData struct
  battery.h         - voltage reading, thresholds
  display_utils.h   - drawing primitives
  joystick_utils.h  - input processing
```

Read all .ino files and produce a duplication analysis.
