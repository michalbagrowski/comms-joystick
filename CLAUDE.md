# AI Catch-Up Guide - ESP32 Dual Joystick BLE Project

**Last Updated:** 2026-01-17
**Status:** Active Development - ADC voltage issue discovered

---

## Quick Project Summary

Wireless dual-joystick controller using BLE between two ESP32-C3 boards:
- **Transmitter (TX)**: Reads 2x HW-504 analog joysticks, broadcasts via BLE at 10Hz
- **Receiver (RX)**: Receives BLE data, displays same joystick positions on OLED

---

## Critical Hardware Configuration

### Transmitter Board
- **Board**: ESP32-C3 Super Mini
- **Joysticks**: 2x HW-504 modules (analog XY + digital button)
  - J1: GPIO0 (VRx), GPIO1 (VRy), GPIO2 (SW)
  - J2: GPIO3 (VRx), GPIO4 (VRy), GPIO5 (SW)
- **Display**: SSD1306 OLED 0.91" 128x32 I2C @ 0x3C
  - GPIO6 (SDA), GPIO7 (SCL)
- **LED**: GPIO8 (built-in, blinks 500ms)

### Receiver Board
- **Board**: ESP32-C3 Super Mini
- **Display**: SSD1306 OLED 0.91" 128x32 I2C @ 0x3C
  - GPIO6 (SDA), GPIO7 (SCL)

### Power Configuration
- **OLED**: 3.3V only (will damage at 5V)
- **Joysticks**: Currently powered by 5V (ISSUE - see below)
- **ESP32-C3 ADC**: 0-3.3V max, 12-bit (0-4095)

---

## Measured Joystick Calibration Values

**At rest (neutral position), measured 2026-01-17:**

| Joystick | X Center | Y Center |
|----------|----------|----------|
| J1       | 3515     | 3234     |
| J2       | 3352     | 3510     |

These values are **device-specific** and stored in code as:
- `JOY1_CENTER_X`, `JOY1_CENTER_Y`
- `JOY2_CENTER_X`, `JOY2_CENTER_Y`

**Previous assumption:** Center was 3200,3200 (incorrect)

---

## Current Issues & Solutions

### ⚠️ CRITICAL ISSUE: ADC Voltage Mismatch

**Problem:**
- Joysticks powered by **5V** but ESP32-C3 ADC only handles **0-3.3V**
- Symptoms:
  - Moving joystick "down": Values decrease slowly from 3500, then accelerate
  - Moving joystick "up": Values jump immediately to 4095 (clipping at 3.3V threshold)
  - Non-linear behavior across range
  - Happens on ALL joysticks (not a hardware defect)

**Root Cause:**
1. HW-504 joystick outputs 0V to VCC (5V in this case)
2. ESP32-C3 ADC clamps input at 3.3V → anything above reads as 4095
3. ESP32-C3 ADC at 11dB attenuation (default) has non-linear response curve

**Solution (RECOMMENDED):**
```
Change wiring: Connect joystick 5V pins to ESP32 3.3V instead of 5V
```

This will:
- Provide proper 0-3.3V output range
- Eliminate clipping at 4095
- Give linear response across full range
- HW-504 works fine at 3.3V according to datasheet

**Alternative (NOT RECOMMENDED):**
Add external voltage divider (5V → 3.3V) but requires additional components.

---

## BLE Communication Protocol

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Update Rate**: 100ms (10 Hz)
- **Data Structure**: 12 bytes packed struct
  ```c
  struct JoystickData {
    int16_t joy1_x;    // 2 bytes
    int16_t joy1_y;    // 2 bytes
    uint8_t joy1_sw;   // 1 byte (LOW=pressed, HIGH=released)
    int16_t joy2_x;    // 2 bytes
    int16_t joy2_y;    // 2 bytes
    uint8_t joy2_sw;   // 1 byte
  } // Total: 12 bytes
  ```

---

## Display Visualization Requirements

**User Request:** Zero state (neutral joystick position) should appear at **bottom-right corner** of circle.

**Current Implementation:**
- Circle drawn at screen positions: J1(32,18), J2(96,18) with radius 14
- Crosshairs at bottom-right corner: `(centerX + radius - 3, centerY + radius - 3)`
- Joystick dot mapped relative to crosshairs position
- When joystick at rest → dot appears at crosshairs (bottom-right)
- Movement radiates from that corner

**Mapping Logic:**
```c
// Zero position at bottom-right corner
int zeroX = centerX + (radius - 3);
int zeroY = centerY + (radius - 3);

// Map joystick offset to screen coordinates
int offsetX = joyX - joyCenterX;
int offsetY = joyY - joyCenterY;
int mapX = map(offsetX, -joyCenterX, 4095 - joyCenterX, -2*(radius - 3), 0);
int mapY = map(offsetY, -joyCenterY, 4095 - joyCenterY, -2*(radius - 3), 0);

// Position relative to zero
int dotX = zeroX + mapX;
int dotY = zeroY + mapY;
```

---

## Debug Mode

**To enable raw value display** (remove circles, show numbers):

In both `transmitter.ino` and `receiver.ino`, comment out `drawJoystick()` calls and add:
```c
display.setTextSize(1);
display.setCursor(0, 0);
display.print("J1: ");
display.print(joystickData.joy1_x);
display.print(",");
display.print(joystickData.joy1_y);

display.setCursor(0, 10);
display.print("J2: ");
display.print(joystickData.joy2_x);
display.print(",");
display.print(joystickData.joy2_y);
```

Use this to:
- Verify joystick center values
- Diagnose ADC clipping issues
- Check BLE transmission accuracy

---

## Build System

### Auto-Port Detection
Makefile auto-detects connected ESP32 boards:
```makefile
TX_PORT = $(shell arduino-cli board list | grep "Serial Port (USB)" | head -n1 | awk '{print $1}')
RX_PORT = $(shell arduino-cli board list | grep "Serial Port (USB)" | tail -n1 | awk '{print $1}')
```

Detected ports (as of 2026-01-17):
- First board: `/dev/cu.usbmodem21101`
- Second board: `/dev/cu.usbmodem21201`

### Key Commands
```bash
make all                  # Compile + upload both boards
make compile              # Compile only
make upload-transmitter   # Upload TX
make upload-receiver      # Upload RX
make list-ports           # Show connected boards
make monitor-transmitter  # Serial monitor @ 115200
make identify             # Blink LED to identify which board is which
```

---

## File Structure

```
/Users/acid/Projects/esp32/comms+joystick/
├── transmitter/
│   └── transmitter.ino       (232+ lines, TX board code)
├── receiver/
│   └── receiver.ino          (234+ lines, RX board code)
├── Makefile                  (Build automation)
├── README.md                 (Main documentation)
├── QUICKSTART.md             (Quick start guide)
├── PIN_CONNECTIONS.txt       (Exact pin mappings)
├── WIRING.txt                (Detailed wiring diagrams)
└── AI_CATCHUP.md             (This file)
```

---

## Code Patterns & Conventions

### Non-Blocking Timing
```c
static unsigned long lastUpdate = 0;
if (millis() - lastUpdate > 100) {
  // Do work every 100ms
  lastUpdate = millis();
}
```

### Per-Joystick Center Values
```c
// Each joystick has unique center position
#define JOY1_CENTER_X 3515
#define JOY1_CENTER_Y 3234
#define JOY2_CENTER_X 3352
#define JOY2_CENTER_Y 3510
```

### Button Active-LOW
```c
uint8_t button = digitalRead(JOY_SW);
// button == LOW  → pressed
// button == HIGH → released
```

### BLE Server Callbacks (TX)
```c
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { deviceConnected = true; }
  void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};
```

### BLE Client Callbacks (RX)
```c
static void notifyCallback(BLERemoteCharacteristic*, uint8_t* pData, size_t length, bool isNotify) {
  if (length == sizeof(joystickData)) {
    memcpy(&joystickData, pData, sizeof(joystickData));
  }
}
```

---

## Known Technical Details

### ADC Configuration
- **Resolution**: 12-bit (0-4095)
- **Attenuation**: ADC_11db (default, 0-3.3V range)
- **Non-linearity**: 11dB attenuation has known non-linear curve on ESP32-C3
- **Recommendation**: Use 3.3V power for joysticks to avoid issues

### I2C Configuration
- **Custom pins**: GPIO6 (SDA), GPIO7 (SCL) - NOT default ESP32 I2C pins
- **OLED address**: 0x3C
- **Display resolution**: 128x32 pixels
- **Refresh rate**: ~100ms (synced with sensor read)

### Direction Detection
```c
String getDirection(int x, int y, int centerX, int centerY) {
  int threshold = 1500;  // Deadzone
  int dx = x - centerX;
  int dy = y - centerY;

  if (abs(dx) < threshold && abs(dy) < threshold) return "CENTER";

  // Priority to larger axis
  if (abs(dx) > abs(dy)) {
    return (dx > 0) ? "RIGHT" : "LEFT";
  } else {
    return (dy > 0) ? "DOWN" : "UP";
  }
}
```

---

## Git Status (2026-01-17)

```
Current branch: main
Modified files:
  M receiver/receiver.ino
  M transmitter/transmitter.ino

Last commit: e8ca6e5 init
```

---

## Development History

### Session 2026-01-17

1. **Initial Analysis**
   - Explored project structure
   - Documented architecture and communication protocol
   - Identified all files and configurations

2. **Display Zero Position Request**
   - User requested zero state at bottom-right corner of circle
   - Modified `drawJoystick()` function to place crosshairs at bottom-right
   - Updated mapping logic to position dot relative to crosshairs

3. **Calibration Discovery**
   - Added debug mode to display raw values
   - Measured actual joystick center positions (not 3200,3200 as assumed)
   - Updated center values per joystick: J1(3515,3234), J2(3352,3510)

4. **ADC Voltage Issue Discovery**
   - User reported non-linear behavior: slow down, instant max up
   - Diagnosed as voltage mismatch: 5V joystick → 3.3V ADC
   - Solution: Change joystick power from 5V to 3.3V pin

5. **Makefile Enhancement**
   - Auto-detect USB ports
   - `make all` now compiles + uploads both boards
   - Added port display in upload targets

---

## Next Steps / TODO

1. **CRITICAL**: Test with joysticks powered by 3.3V instead of 5V
2. Verify ADC linearity after voltage change
3. Re-measure center values if needed (may shift with 3.3V)
4. Re-enable circle display visualization with corrected calibration
5. Test full joystick range in all directions
6. Verify BLE transmission accuracy across full range
7. Optional: Add smoothing/filtering for jitter reduction

---

## Troubleshooting Quick Reference

### Problem: Values jump to 4095 immediately
**Solution**: Joysticks powered by 5V, change to 3.3V

### Problem: Joystick center not at crosshairs
**Solution**: Enable debug mode, measure actual center values, update constants

### Problem: Can't upload to boards
**Solution**: Run `make list-ports` to verify connections, check TX_PORT/RX_PORT

### Problem: BLE not connecting
**Solution**: Wait 10 seconds, check serial monitor, verify both boards powered

### Problem: Display shows wrong joystick
**Solution**: Run `make identify` to blink first board LED, verify physical layout

### Problem: Non-linear joystick response
**Solution**: Verify 3.3V power, check ADC attenuation setting

---

## References

- [ESP32-C3 ADC Documentation](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32c3/api-reference/peripherals/adc.html)
- [ESP32 ADC Non-linear Issues](https://www.esp32.com/viewtopic.php?t=2881)
- [HW-504 Joystick Specifications](https://components101.com/modules/joystick-module)
- [Arduino ESP32 ADC Documentation](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/adc.html)

---

## Important Notes for Future Sessions

1. **User prefers:** Numbers displayed clearly for debugging, circles for visualization
2. **Zero position:** Must be at bottom-right corner (user requirement)
3. **Multiple joysticks tested:** Issue is not hardware defect, it's voltage mismatch
4. **Center values are device-specific:** Always measure, don't assume
5. **ADC non-linearity:** Known ESP32-C3 issue at 11dB attenuation with >3.3V input

---

*This file serves as a comprehensive catch-up guide. Update it after significant changes or discoveries.*
