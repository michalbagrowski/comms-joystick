# AI Catch-Up Guide - ESP32 Dual Joystick BLE/WiFi Project

**Last Updated:** 2026-01-19
**Status:** WiFi mode added, compile-time switching implemented, dual communication modes supported

---

## Quick Project Summary

Wireless dual-joystick controller with **two communication modes** between ESP32-C3 boards:

### BLE Mode (Default)
- **Transmitter (TX)**: Reads 2x HW-504 analog joysticks, broadcasts via BLE at 10Hz
- **Receiver (RX)**: Receives BLE data, displays joystick positions + controls servo motor

### WiFi Mode (Optional)
- **Transmitter (TX)**: Reads 2x HW-504 analog joysticks, broadcasts via UDP/WiFi at 10Hz
- **Receiver (RX)**: Receives UDP packets, displays joystick positions + controls servo motor
- **Network**: Connects to "amplifi" WiFi network
- **Discovery**: Automatic via mDNS (esp32-joystick-tx.local)

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
- **Servo**: SG90 mini servo on GPIO0
  - Controls based on Joystick 1 X-axis
  - Range: 0° to 180°

### Power Configuration
- **OLED**: 3.3V only (will damage at 5V)
- **Joysticks**: 3.3V (changed from 5V - RESOLVED)
- **Servo**: 5V from ESP32 or external supply
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

## Issues Discovered & Solutions Implemented

### ✅ RESOLVED: ADC Voltage Mismatch

**Problem (Discovered):**
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

**Solution (IMPLEMENTED):**
```
Changed wiring: Joystick 5V pins → ESP32 3.3V pin
```

**Result:** Better linearity, but still some noise/jitter

### ✅ IMPLEMENTED: Software Filtering

**ADC Noise Issues:**
- Jitter (±1 value fluctuation) when joystick at rest
- Some instability at extremes
- Values not perfectly smooth

**Solution (IMPLEMENTED):**
1. **Oversampling**: Read ADC 4 times and average (reduces random noise)
2. **Exponential Moving Average**: Smooth readings over time with alpha=0.3
3. **Result**: Much smoother, more stable readings

**Code Implementation (transmitter.ino):**
```c
#define ADC_SAMPLES 4
#define SMOOTHING_ALPHA 0.3

int readADC(int pin) {
  long sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  return sum / ADC_SAMPLES;
}

// Then apply exponential moving average
filtered_value = (SMOOTHING_ALPHA * raw_value) + ((1.0 - SMOOTHING_ALPHA) * filtered_value);
```

**Hardware Filtering (If Needed):**
- See `HARDWARE_FILTERING.md` for capacitor/RC filter options
- Add 0.1µF capacitors between ADC pins and GND if software filtering insufficient

### ✅ RESOLVED: Receiver Display Dimness

**Problem:** Receiver OLED appeared dimmer than transmitter

**Solution:** Added explicit brightness control to both displays
```c
display.ssd1306_command(0x81); // Set contrast control
display.ssd1306_command(0xFF); // Maximum brightness (255)
```

**Also check:** Power supply voltage (should be 3.25-3.35V at OLED VCC pin)

---

## Communication Modes

The project supports **two communication modes** selectable at compile-time:

### BLE Mode (Default)

**When to use:** Simple setup, low power, short range (10-30m)

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **Update Rate**: 100ms (10 Hz)
- **Range**: 10-30m
- **Power**: Low
- **Setup**: Automatic pairing

### WiFi Mode (Optional)

**When to use:** Longer range (50-100m), lower latency, existing WiFi network

- **Protocol**: UDP
- **Port**: 4210
- **Network**: "amplifi" (SSID: "amplifi", Password: "123qwe123")
- **Discovery**: mDNS (esp32-joystick-tx.local) with broadcast fallback
- **Update Rate**: 100ms (10 Hz)
- **Range**: 50-100m
- **Power**: Higher
- **Setup**: Requires WiFi network

### Data Structure (Same for Both Modes)

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

### Switching Between Modes

**To enable WiFi mode:**
1. Edit both `transmitter/transmitter.ino` and `receiver/receiver.ino`
2. Uncomment: `#define USE_WIFI` (line ~5)
3. Compile and upload: `make wifi-all`

**To return to BLE mode:**
1. Comment out: `// #define USE_WIFI`
2. Compile and upload: `make all`

**No other changes required** - pin assignments, servo control, and display layout remain the same.

---

## Display Visualization

**User Request:** Zero state (neutral joystick position) should appear at **bottom-right corner** of circle.

### Transmitter Display (128x32 pixels)
```
┌──────────────────────────────────────────────────┐
│ J1    ○       J2    ○               TX          │  ← Top: Circles + status
│       /│\            /│\                         │
│      ┼─○            ┼─○                          │  ← Crosshairs at bottom-right
│                                                  │
│ 3515               3352                          │  ← Bottom: Raw X values
└──────────────────────────────────────────────────┘
```
- **Two joystick circles**: J1(25,12) radius 10, J2(75,12) radius 10
- **Crosshairs at bottom-right corner** of each circle (zero position)
- **Joystick dot** position relative to crosshairs
- **Status indicator**:
  - BLE mode: "TX" or "--" at top-right (110,0)
  - WiFi mode: "WiFi" or "----" at top-right (98,0)
- **Raw values**: Bottom shows X values for both joysticks

### Receiver Display (128x32 pixels)
```
┌──────────────────────────────────────────────────┐
│ J1  ○     J2  ○                     RX          │  ← Top: Circles + status
│     /│\        /│\                               │
│    ┼─○        ┼─○                                │
│ S:90°                                            │  ← Servo angle
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░  │  ← Servo bar (0-180°)
└──────────────────────────────────────────────────┘
```
- **Two joystick circles**: J1(25,10) radius 8, J2(75,10) radius 8
- **Servo position bar**: Bottom (y=28-31), fills left-to-right based on angle
- **Servo angle text**: "S:XX°" above bar (0,20)
- **Status indicator**:
  - BLE mode: "RX" at top-right (110,0)
  - WiFi mode: "WiFi" at top-right (98,0)

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

## Servo Control

**Configuration:**
- **Pin**: GPIO0 (receiver)
- **Control Source**: Joystick 1 X-axis
- **Mapping**: Linear 0-4095 → 0-180°
- **Library**: ESP32Servo

**Code (receiver.ino):**
```c
int servoAngle = map(joystickData.joy1_x, 0, 4095, 0, 180);
servoAngle = constrain(servoAngle, 0, 180);
myServo.write(servoAngle);
```

**To change control source:** Edit receiver.ino ~line 247
- Joy1 Y-axis: `joystickData.joy1_y`
- Joy2 X-axis: `joystickData.joy2_x`
- Joy2 Y-axis: `joystickData.joy2_y`

**Wiring:**
- Brown (GND) → ESP32 GND
- Red (VCC) → ESP32 5V (or external 5V supply for heavy loads)
- Orange/Yellow (Signal) → ESP32 GPIO0

**See:** `SERVO_SETUP.md` for complete wiring and configuration guide

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

**BLE Mode (Default):**
```bash
make all                  # Compile + upload both boards (BLE)
make compile              # Compile only (BLE)
make install-libs         # Install all libraries (includes ESP32Servo)
make upload-transmitter   # Upload TX
make upload-receiver      # Upload RX
make list-ports           # Show connected boards
make monitor-transmitter  # Serial monitor @ 115200
make identify             # Blink LED to identify which board is which
```

**WiFi Mode:**
```bash
make wifi-all                  # Compile + upload both boards (WiFi)
make wifi-compile              # Compile only (WiFi)
make wifi-compile-transmitter  # Compile TX only (WiFi)
make wifi-compile-receiver     # Compile RX only (WiFi)
make wifi-upload-transmitter   # Upload TX (WiFi)
make wifi-upload-receiver      # Upload RX (WiFi)
```

**Note:** WiFi mode requires uncommenting `#define USE_WIFI` in both `.ino` files first.

---

## File Structure

```
/Users/acid/Projects/esp32/comms+joystick/
├── transmitter/
│   └── transmitter.ino       (370+ lines, TX with WiFi/BLE)
├── receiver/
│   └── receiver.ino          (420+ lines, RX with WiFi/BLE + servo)
├── Makefile                  (Build automation with WiFi targets)
├── README.md                 (Main documentation)
├── QUICKSTART.md             (Quick start guide)
├── PIN_CONNECTIONS.txt       (Exact pin mappings)
├── WIRING.txt                (Detailed wiring diagrams)
├── HARDWARE_FILTERING.md     (Capacitor/RC filter guide)
├── SERVO_SETUP.md            (Servo wiring and configuration)
├── WIFI_SETUP.md             (WiFi configuration and troubleshooting)
└── CLAUDE.md                 (This file - AI catch-up guide)
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

### Filtering Implementation
```c
// Oversampling
int readADC(int pin) {
  long sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  return sum / ADC_SAMPLES;
}

// Exponential moving average
filtered_j1x = (SMOOTHING_ALPHA * raw_j1x) + ((1.0 - SMOOTHING_ALPHA) * filtered_j1x);
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

---

## Known Technical Details

### ADC Configuration
- **Resolution**: 12-bit (0-4095)
- **Attenuation**: ADC_11db (default, 0-3.3V range)
- **Non-linearity**: 11dB attenuation has known non-linear curve on ESP32-C3
- **Power**: Joysticks now on 3.3V (changed from 5V)
- **Filtering**: Oversampling (4 samples) + exponential moving average (alpha=0.3)

### Servo Configuration
- **Model**: SG90 mini servo
- **Pin**: GPIO0 (receiver)
- **Control**: Joystick 1 X-axis (linear mapping 0-4095 → 0-180°)
- **Power**: 5V from ESP32 or external supply
- **Library**: ESP32Servo

### I2C Configuration
- **Custom pins**: GPIO6 (SDA), GPIO7 (SCL) - NOT default ESP32 I2C pins
- **OLED address**: 0x3C
- **Display resolution**: 128x32 pixels
- **Brightness**: Set to maximum (255) on both displays
- **Refresh rate**: ~100ms (synced with sensor read)

---

## Git Status (2026-01-17)

```
Current branch: main
Modified files:
  M receiver/receiver.ino
  M transmitter/transmitter.ino
  M Makefile

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

4. **ADC Voltage Issue Discovery & Resolution**
   - User reported non-linear behavior: slow down, instant max up
   - Diagnosed as voltage mismatch: 5V joystick → 3.3V ADC
   - Solution: Changed joystick power from 5V to 3.3V pin
   - Result: Better but still some jitter

5. **Software Filtering Implementation**
   - Added oversampling: Read ADC 4 times and average
   - Added exponential moving average filter (alpha=0.3)
   - Created `HARDWARE_FILTERING.md` guide for capacitor options
   - Result: Much smoother, more stable readings

6. **Receiver Display Dimness**
   - User reported receiver OLED dimmer than transmitter
   - Added explicit brightness control (0x81 command, 0xFF value)
   - Set both displays to maximum brightness (255)
   - May also be power supply issue (check 3.3V voltage)

7. **Servo Control Addition**
   - Added SG90 servo to receiver on GPIO0
   - Controls based on Joystick 1 X-axis (0-4095 → 0-180°)
   - Added ESP32Servo library to dependencies
   - Created `SERVO_SETUP.md` guide with wiring and configuration

8. **Visual Representation Enhancement**
   - Added visual joystick circles to both displays
   - Added servo position bar to receiver display
   - Compact layout: Circles + status + info in 128x32 pixels
   - Transmitter: Two circles + raw values at bottom
   - Receiver: Two circles + servo bar + servo angle text

9. **Makefile Enhancement**
   - Auto-detect USB ports
   - `make all` now compiles + uploads both boards
   - Added ESP32Servo to install-libs target
   - Added port display in upload targets

### Session 2026-01-19

10. **WiFi Communication Mode Implementation**
   - Added compile-time switch between BLE and WiFi modes
   - Implemented UDP communication on port 4210
   - Added mDNS discovery (esp32-joystick-tx.local) with broadcast fallback
   - Network configuration: SSID "amplifi", password "123qwe123"
   - All existing functionality preserved (servo, display, filtering)

11. **Conditional Compilation Structure**
   - Added `#define USE_WIFI` switch at top of both files
   - Wrapped BLE code in `#ifndef USE_WIFI`
   - Wrapped WiFi code in `#ifdef USE_WIFI`
   - Same data structure (12 bytes) for both modes
   - Display status shows "WiFi" or "TX"/"RX" based on mode

12. **Makefile WiFi Targets**
   - Added `wifi-all` target for complete WiFi build
   - Added `wifi-compile`, `wifi-compile-transmitter`, `wifi-compile-receiver`
   - Added `wifi-upload-transmitter`, `wifi-upload-receiver`
   - Used `compiler.cpp.extra_flags=-DUSE_WIFI` for proper compilation
   - Updated help text to document WiFi targets

13. **Comprehensive Documentation**
   - Created `WIFI_SETUP.md` (complete WiFi guide)
   - Updated `CLAUDE.md` with WiFi sections
   - Added troubleshooting for WiFi-specific issues
   - Documented switching between BLE and WiFi modes
   - Added performance comparison table

14. **Testing & Verification**
   - BLE mode compiles: 717KB TX, 749KB RX (54-57% flash)
   - WiFi mode compiles: 1041KB TX, 1023KB RX (78-79% flash)
   - Both modes verified to compile without errors
   - No changes required to pin assignments or hardware

---

## Next Steps / TODO

1. ✅ ~~Test with joysticks powered by 3.3V~~ - DONE
2. ✅ ~~Add software filtering~~ - DONE (oversampling + EMA)
3. ✅ ~~Add servo control to receiver~~ - DONE (GPIO0, Joy1 X-axis)
4. ✅ ~~Add visual representation~~ - DONE (circles + servo bar)
5. ✅ ~~Add WiFi communication mode~~ - DONE (UDP, mDNS, compile-time switch)
6. Test full joystick range with filtering in all directions (both modes)
7. Verify servo responds smoothly to filtered joystick input (both modes)
8. Test WiFi mode with actual network (range, latency, reliability)
9. Optional: Add hardware filtering (capacitors) if software filtering insufficient
10. Optional: Add multiple servos on different GPIO pins
11. Optional: Add servo smoothing/deadband on receiver side
12. Optional: Add runtime mode switching (WiFi/BLE toggle)
13. Optional: Add packet sequencing and timeout detection for WiFi mode

---

## Troubleshooting Quick Reference

### Problem: Values jump to 4095 immediately
**Solution**: Joysticks powered by 5V, change to 3.3V (RESOLVED)

### Problem: Joystick center not at crosshairs
**Solution**: Enable debug mode, measure actual center values, update constants (RESOLVED)

### Problem: Values jitter ±1 when stationary
**Solution**: Software filtering enabled (oversampling + EMA), add capacitors if needed

### Problem: Can't upload to boards
**Solution**: Run `make list-ports` to verify connections, check TX_PORT/RX_PORT

### Problem: BLE not connecting
**Solution**: Wait 10 seconds, check serial monitor, verify both boards powered

### Problem: Display shows wrong joystick
**Solution**: Run `make identify` to blink first board LED, verify physical layout

### Problem: Non-linear joystick response
**Solution**: Verify 3.3V power, check ADC attenuation setting, software filtering enabled (RESOLVED)

### Problem: Servo jitters or doesn't move smoothly
**Solution**: Check wiring, verify power (5V), add capacitor filtering, check BLE connection stable

### Problem: Receiver display dimmer than transmitter
**Solution**: Brightness now set to max (255), check 3.3V voltage at OLED, verify wiring quality

### Problem: Servo doesn't move
**Solution**: Check signal wire to GPIO0, verify 5V power, see `SERVO_SETUP.md`

### Problem: ESP32 resets when servo moves
**Solution**: Use external 5V power supply for servo (insufficient current from USB)

### Problem: WiFi connection failed
**Solution**: Verify SSID/password in code, ensure 2.4GHz WiFi enabled, check signal strength, see `WIFI_SETUP.md`

### Problem: Transmitter discovery failed (WiFi mode)
**Solution**: Router may block mDNS, check "Client Isolation" disabled, receiver will auto-fallback to broadcast mode

### Problem: No data received (WiFi mode)
**Solution**: Verify both boards on same network, check Serial monitor for IP addresses, ensure firewall allows UDP port 4210

### Problem: WiFi mode compilation fails
**Solution**: Ensure using `make wifi-compile` (not manual arduino-cli with wrong flags), see Makefile for correct build property

---

## References

### Hardware & ADC
- [ESP32-C3 ADC Documentation](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32c3/api-reference/peripherals/adc.html)
- [ESP32 ADC Non-linear Issues](https://www.esp32.com/viewtopic.php?t=2881)
- [HW-504 Joystick Specifications](https://components101.com/modules/joystick-module)
- [Arduino ESP32 ADC Documentation](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/adc.html)

### Libraries
- [ESP32Servo Library](https://github.com/madhephaestus/ESP32Servo)
- [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)

### WiFi & Networking
- [ESP32 WiFi Library Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
- [ESP32 mDNS Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/mdns.html)
- [UDP Protocol Overview](https://en.wikipedia.org/wiki/User_Datagram_Protocol)
- [mDNS Protocol Specification](https://en.wikipedia.org/wiki/Multicast_DNS)

---

## Important Notes for Future Sessions

1. **User prefers:** Visual circles for operation, numbers for debugging
2. **Zero position:** Must be at bottom-right corner (user requirement)
3. **Multiple joysticks tested:** Issue is not hardware defect, it's voltage mismatch (RESOLVED)
4. **Center values are device-specific:** Always measure, don't assume
5. **ADC non-linearity:** Known ESP32-C3 issue at 11dB attenuation with >3.3V input (RESOLVED)
6. **Filtering is essential:** Software filtering implemented, hardware filtering optional
7. **Servo control:** Currently Joy1 X-axis, easily configurable to other axes
8. **Communication modes:** Two modes (BLE default, WiFi optional), compile-time switch via `#define USE_WIFI`
9. **WiFi network:** Default "amplifi" / "123qwe123", easily changed in code
10. **Build flags:** Use `compiler.cpp.extra_flags` not `build.extra_flags` for WiFi mode compilation
11. **Mode switching:** No hardware changes needed, just recompile and upload

---

*This file serves as a comprehensive catch-up guide. Update it after significant changes or discoveries.*
