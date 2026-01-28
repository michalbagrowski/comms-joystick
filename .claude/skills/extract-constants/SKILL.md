---
name: extract-constants
description: Find magic numbers in ESP32 code and propose named constants for better maintainability
allowed-tools: Read, Grep, Glob
---

# Extract Magic Numbers to Constants

Find and extract hardcoded values into named constants for better maintainability.

## Target Values to Extract

### Hardware Configuration
- GPIO pin numbers
- I2C addresses (0x3C)
- ADC range (0-4095)
- PWM frequencies and resolutions

### Display Coordinates
- Screen dimensions (128x64)
- Joystick visualization positions
- Text positions
- Bar graph coordinates

### Timing Values
- Update intervals (100ms, 1000ms)
- LED blink periods
- Timeout durations
- Debounce times

### Control Parameters
- Servo range (0-180)
- Motor speed range (-255 to 255)
- Deadzone thresholds
- Joystick center values

### Battery Thresholds
- Voltage levels (4.2V, 3.5V, 3.3V, 3.2V)
- Divider ratio (2.0)
- Warning/critical thresholds

### Communication
- BLE UUIDs
- WiFi credentials
- UDP port
- Update rates

## Process

1. Scan all .ino files for numeric literals
2. Identify repeated values
3. Group by category
4. Suggest constant names following convention:
   - SCREAMING_SNAKE_CASE for #define
   - kCamelCase for const
5. Show before/after code

## Output Format

```cpp
// === PROPOSED CONSTANTS ===

// Hardware Pins
#define PIN_JOY1_X      0
#define PIN_JOY1_Y      1
...

// Display Configuration
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
...

// Timing (milliseconds)
#define DISPLAY_UPDATE_MS   100
#define BATTERY_CHECK_MS    1000
...

// Battery Thresholds (volts)
#define VBAT_FULL       4.2f
#define VBAT_LOW        3.5f
...
```

Then show which lines would change in each file.

Review all .ino files and produce a constants extraction plan.
