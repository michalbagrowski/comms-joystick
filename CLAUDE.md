# AI Catch-Up Guide - ESP32 Dual Joystick BLE/WiFi Project

**Last Updated:** 2026-01-21
**Status:** Display drivers corrected, joystick calibration updated, USB CDC enabled

---

## Quick Project Summary

Wireless dual-joystick controller with **DC motors** and **two communication modes** between ESP32-C3 boards:

### BLE Mode (Default)
- **Transmitter (TX)**: Reads 2x HW-504 analog joysticks, broadcasts via BLE at 10Hz, displays "fake motor speeds"
- **Receiver (RX)**: Receives BLE data, displays joystick positions + controls 2x DC motors via MX1508 driver

### WiFi Mode (Optional)
- **Transmitter (TX)**: Reads 2x HW-504 analog joysticks, broadcasts via UDP/WiFi at 10Hz, displays "fake motor speeds"
- **Receiver (RX)**: Receives UDP packets, displays joystick positions + controls 2x DC motors via MX1508 driver
- **Network**: Connects to "amplifi" WiFi network
- **Discovery**: Automatic via mDNS (esp32-joystick-tx.local)

**Key Features:**
- **Joy1** controls servo: Joy1 X-axis → Servo angle (0-180°)
- **Joy2** controls DC motors:
  - Joy2 X-axis → Motor 1 (left/right movement)
  - Joy2 Y-axis → Motor 2 (forward/backward movement)

---

## Critical Hardware Configuration

### Transmitter Board
- **Board**: ESP32-C3 Super Mini
- **Joysticks**: 2x HW-504 modules (analog XY + digital button)
  - J1: GPIO0 (VRx), GPIO1 (VRy), GPIO2 (SW)
  - J2: GPIO3 (VRx), GPIO4 (VRy), GPIO5 (SW)
- **Display**: 1.9" OLED 128x64 I2C (SH1106) @ 0x3C
  - GPIO6 (SDA), GPIO7 (SCL)
  - Library: Adafruit_SH110X
  - Shows joystick positions + "fake motor speeds" (calculated from Joy2)
- **LED**: GPIO8 (built-in, blinks every 1 second)

### Receiver Board
- **Board**: ESP32-C3 Super Mini
- **Display**: 1.3" OLED 128x64 I2C (SSD1306) @ 0x3C
  - GPIO6 (SDA), GPIO7 (SCL)
  - Library: Adafruit_SSD1306
  - Shows joystick positions + servo angle + motor speeds (as bars)
- **LED**: GPIO8 (built-in, blinks every 3 seconds)
- **Servo**: SG90 mini servo on GPIO4
  - Controls based on Joystick 1 X-axis (0-4095 → 0-180°)
  - Independent angular control (e.g., steering, camera pan)
- **Motor Driver**: MX1508 dual H-bridge
  - Motor 1 (Joy2 X-axis): GPIO0 (IN1), GPIO1 (IN2)
  - Motor 2 (Joy2 Y-axis): GPIO2 (IN3), GPIO3 (IN4)
  - PWM: 20kHz, 8-bit resolution (0-255)
  - **Motor Enable**: GPIO5 controls 2N2222 transistor (prevents boot twitch)
- **Motors**: 2x DC motors (3.7V rated)
- **LED**: GPIO8 (built-in, status indicator)

### Power Configuration
- **OLED Displays**: 3.3V only (will damage at 5V)
- **Joysticks**: 3.3V (changed from 5V - RESOLVED)
- **Servo**: 5V (SG90 draws 100-500mA depending on load)
- **MX1508 Driver**: 5V (powers motors)
- **DC Motors**: 3.7V nominal (powered through MX1508 from 5V)
- **ESP32-C3 ADC**: 0-3.3V max, 12-bit (0-4095)
- **Recommended Power**: 5V 3A wall adapter for receiver (servo + motors draw significant current)

---

## Measured Joystick Calibration Values

**At rest (neutral position), measured 2026-01-21:**

| Joystick | X Center | Y Center |
|----------|----------|----------|
| J1       | 2235     | 2217     |
| J2       | 2215     | 2255     |

These values are **device-specific** and stored in code as:
- `JOY1_CENTER_X`, `JOY1_CENTER_Y`
- `JOY2_CENTER_X`, `JOY2_CENTER_Y`

**Note:** Previous values (~3500) were incorrect. Actual centers are ~2220.

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

### Transmitter Display (128x64 pixels - upgraded from 128x32)
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

### Receiver Display (128x64 pixels - upgraded from 128x32)
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

## Servo and Motor Control

### Servo Control (Joy1 X-axis)

**Configuration:**
- **Model**: SG90 mini servo (or compatible)
- **Pin**: GPIO4 (receiver)
- **Control Source**: Joystick 1 X-axis
- **Mapping**: Linear 0-4095 → 0-180°
- **Library**: ESP32Servo
- **Use Cases**: Steering, camera pan, angular positioning

**Code (receiver.ino):**
```c
servo_angle = map(joystickData.joy1_x, 0, 4095, 0, 180);
servo_angle = constrain(servo_angle, 0, 180);
myServo.write(servo_angle);
```

**Power:**
- 100-500mA depending on load
- Can use ESP32 5V pin for light loads
- For heavy loads, use external 5V supply

### DC Motor Control (Joy2 X/Y axes)

**Configuration:**
- **Motor Driver**: MX1508 dual H-bridge
- **Pins**: GPIO0-3 (receiver)
  - Motor 1: GPIO0 (IN1 forward), GPIO1 (IN2 reverse)
  - Motor 2: GPIO2 (IN3 forward), GPIO3 (IN4 reverse)
- **Control Source**: Joystick 2 (Joy2) controls both motors
  - Joy2 X-axis → Motor 1 (left/right)
  - Joy2 Y-axis → Motor 2 (forward/backward)
- **PWM**: ESP32 LEDC (built-in), 20kHz, 8-bit (0-255)
- **Deadzone**: ±200 ADC counts around center (prevents jitter)

**Code (receiver.ino):**
```c
void controlMotors(int16_t joy2_x, int16_t joy2_y) {
  // Motor 1 from Joy2 X-axis
  int motor1_speed = map(joy2_x, 0, 4095, -255, 255);
  // Motor 2 from Joy2 Y-axis
  int motor2_speed = map(joy2_y, 0, 4095, -255, 255);

  // Bidirectional PWM control
  if (motor1_speed > 0) {
    ledcWrite(0, motor1_speed);  // Forward
    ledcWrite(1, 0);
  } else if (motor1_speed < 0) {
    ledcWrite(0, 0);
    ledcWrite(1, -motor1_speed); // Reverse
  } else {
    ledcWrite(0, 0); ledcWrite(1, 0);  // Stop
  }
  // Same for motor 2...
}
```

**Wiring:**
- Servo: Brown/Black (GND) → ESP32 GND, Red (VCC) → ESP32 5V, Orange/Yellow (Signal) → GPIO4
- MX1508 VCC → ESP32 5V
- MX1508 GND → ESP32 GND
- MX1508 IN1 → GPIO0, IN2 → GPIO1, IN3 → GPIO2, IN4 → GPIO3
- Motors connected to MX1508 OUT1-OUT2 and OUT3-OUT4

**Power Considerations:**
- Servo draws 100-500mA depending on load
- Motors can draw 1-2A each under load
- Use 5V 3A wall adapter (USB computer port insufficient)
- Add 1000µF capacitor across power for stability

**See:** `WIRING.md` for complete wiring guide (TX and RX sections)

---

## Build System

### Auto-Port Detection
Makefile auto-detects connected ESP32 boards:
```makefile
TX_PORT = $(shell arduino-cli board list | grep "Serial Port (USB)" | head -n1 | awk '{print $1}')
RX_PORT = $(shell arduino-cli board list | grep "Serial Port (USB)" | tail -n1 | awk '{print $1}')
```

Detected ports (as of 2026-01-21):
- First board: `/dev/cu.usbmodem21101` (TX)
- Second board: `/dev/cu.usbmodem21201` (RX)

### USB CDC On Boot (CRITICAL)
ESP32-C3 requires `CDCOnBoot=cdc` flag for Serial output to work over USB:
```makefile
BOARD_FQBN = esp32:esp32:esp32c3:CDCOnBoot=cdc
```
Without this, Serial.println() output will NOT appear in the serial monitor.

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
│   └── transmitter.ino       (500+ lines, TX with WiFi/BLE + motor speed viz)
├── receiver/
│   └── receiver.ino          (580+ lines, RX with WiFi/BLE + DC motors)
├── Makefile                  (Build automation with WiFi targets)
├── README.md                 (Main documentation)
├── QUICKSTART.md             (Quick start guide)
├── WIRING.md                 (★ COMPLETE wiring guide - TX & RX sections)
├── PIN_CONNECTIONS.txt       (Exact pin mappings)
├── HARDWARE_FILTERING.md     (Capacitor/RC filter guide)
├── WIFI_SETUP.md             (WiFi configuration and troubleshooting)
└── CLAUDE.md                 (This file - AI catch-up guide)
```

**Note:** `WIRING.md` is the comprehensive wiring guide with clear TX and RX sections, step-by-step instructions, troubleshooting, and BOM.

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
- **Display resolution**: 128x64 pixels (upgraded from 128x32)
- **Brightness**: Set to maximum (255) on both displays
- **Refresh rate**: ~100ms (synced with sensor read)
- **Note**: Display pin labels vary - SCX = SCL = SCK (all mean I2C Clock)

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

### Session 2026-01-20

15. **Display Upgrade to 128x64**
   - Transmitter: Upgraded from 0.91" 128x32 to 1.9" 128x64 OLED (SSD1306 compatible)
   - Receiver: Upgraded from 0.91" 128x32 to 1.3" 128x64 OLED (SH1106)
   - Changed `SCREEN_HEIGHT` from 32 to 64 in both files
   - Enlarged joystick circles: radius 10→14, repositioned for better visibility
   - Updated display library for receiver: Adafruit_SSD1306 → Adafruit_SH110X
   - Updated color constants: SSD1306_WHITE → SH110X_WHITE (receiver only)

16. **DC Motor Control Implementation (Replaced Servo)**
   - Removed servo control completely from receiver
   - Added MX1508 dual H-bridge motor driver support
   - Motor 1 (Joy2 X-axis): GPIO0 (IN1), GPIO1 (IN2)
   - Motor 2 (Joy2 Y-axis): GPIO2 (IN3), GPIO3 (IN4)
   - Implemented bidirectional PWM control (forward/reverse)
   - PWM configuration: 20kHz frequency, 8-bit resolution (0-255)
   - Added motor deadzone (±200 ADC counts) to prevent jitter at center
   - Used ESP32 LEDC (built-in PWM), no external motor library needed

17. **Motor Speed Visualization**
   - Transmitter: Added "fake motor speed" calculation and display based on Joy2 values
   - Both displays now show motor speeds as bidirectional horizontal bars
   - M1 bar (y=48-55): Shows Motor 1 speed, center at x=64, arrows show direction
   - M2 bar (y=56-63): Shows Motor 2 speed, center at x=64, arrows show direction
   - Transmitter shows calculated speeds, receiver shows actual PWM values
   - Visual feedback allows user to see motor control response in real-time

18. **Library Dependencies Update**
   - Added `Adafruit SH110X` library to Makefile install-libs target
   - Removed ESP32Servo dependency (no longer needed)
   - Noted that ESP32 LEDC (built-in PWM) requires no external libraries

19. **Comprehensive Wiring Documentation**
   - Consolidated wiring files into single `WIRING.md` with clear TX/RX sections
   - Documented MX1508 motor driver connection details
   - Added power distribution requirements (5V 2A+ recommended for receiver)
   - Included troubleshooting guide for motor issues
   - Added Bill of Materials (BOM) with all components
   - Documented pin assignments for both boards
   - Added testing procedure for motors and displays

20. **Code Architecture Improvements**
   - Receiver: Added `controlMotors()` function for clean motor control
   - Receiver: Added `drawMotorBars()` function for motor speed visualization
   - Transmitter: Added `drawFakeMotorSpeeds()` function to show expected motor behavior
   - Maintained same 100ms update rate for smooth operation
   - Both communication modes (BLE/WiFi) fully compatible with motor control

21. **Servo Restored (User Request)**
   - User requested servo back alongside motors (not replacement)
   - Servo added on GPIO4 (receiver)
   - Joy1 X-axis controls servo (0-4095 → 0-180°)
   - Joy2 X/Y axes control motors (independent control)
   - Added `drawServoBar()` function to both TX and RX displays
   - Transmitter shows "fake" servo angle based on Joy1 X-axis
   - Receiver shows actual servo position + motor speeds
   - Display layout: Joystick circles (top) → Servo bar (middle) → Motor bars (bottom)
   - Updated WIRING.md with servo wiring steps
   - Power budget updated: 5V 3A recommended (servo + motors)

### Session 2026-01-21

22. **USB CDC On Boot Fix**
   - Serial output was not appearing over USB
   - Root cause: ESP32-C3 requires `CDCOnBoot=cdc` board option
   - Updated Makefile: `BOARD_FQBN = esp32:esp32:esp32c3:CDCOnBoot=cdc`
   - Serial.println() now works correctly

23. **Display Driver Correction**
   - TX display was showing artifacts with SSD1306 driver
   - RX display was not working with SH1106 driver
   - **Corrected configuration:**
     - TX display: Uses **SH1106** driver (Adafruit_SH110X)
     - RX display: Uses **SSD1306** driver (Adafruit_SSD1306)
   - Both displays now work correctly

24. **Joystick Calibration Update**
   - Display visualization was incorrect (axes appearing on wrong circles)
   - Created joystick diagnostic tool to measure actual GPIO values
   - **Old center values:** ~3500 (incorrect)
   - **New center values:** ~2220 (correct)
   - Updated both TX and RX code with correct values:
     - JOY1_CENTER_X: 2235, JOY1_CENTER_Y: 2217
     - JOY2_CENTER_X: 2215, JOY2_CENTER_Y: 2255

25. **LED Blink Rates Configured**
   - TX: Blinks every 1 second
   - RX: Blinks every 3 seconds
   - Allows visual identification of which board is which

26. **WiFi Mode Disabled by Default**
   - Commented out `#define USE_WIFI` in both files
   - BLE mode now default (doesn't require WiFi network)

---

## Next Steps / TODO

1. ✅ ~~Test with joysticks powered by 3.3V~~ - DONE
2. ✅ ~~Add software filtering~~ - DONE (oversampling + EMA)
3. ✅ ~~Add servo control to receiver~~ - REPLACED with DC motors
4. ✅ ~~Add visual representation~~ - DONE (circles + motor bars on 128x64 displays)
5. ✅ ~~Add WiFi communication mode~~ - DONE (UDP, mDNS, compile-time switch)
6. ✅ ~~Upgrade displays to 128x64~~ - DONE (1.9" TX, 1.3" SH1106 RX)
7. ✅ ~~Add DC motor control~~ - DONE (MX1508, Joy2 controls both motors)
8. Test full joystick range with filtering in all directions (both modes)
9. Test motor response with different joystick positions (both motors, both directions)
10. Test WiFi mode with actual network (range, latency, reliability)
11. Verify motor speed bars on TX match actual motor speeds on RX
12. Test power requirements (motors may need external 5V 2A+ supply)
13. Optional: Add hardware filtering (capacitors) if software filtering insufficient
14. Optional: Add motor ramping/acceleration curves for smoother control
15. Optional: Add motor current sensing for overload detection
16. Optional: Add runtime mode switching (WiFi/BLE toggle)
17. Optional: Add packet sequencing and timeout detection for WiFi mode
18. Optional: Add actual speed measurements (RPM) using encoders (future enhancement)

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

### Problem: Motors don't spin
**Solution**: Check MX1508 power (5V), verify GPIO0-3 connections, check motor wiring to OUT1-4, test TX Joy2 joystick

### Problem: Motors spin weakly
**Solution**: Insufficient power - use 5V 2A+ adapter, add 1000µF capacitor, check motor voltage rating

### Problem: Receiver display dimmer than transmitter
**Solution**: Brightness now set to max (255), check 3.3V voltage at OLED, verify wiring quality

### Problem: Motor spins wrong direction
**Solution**: Swap motor wires at MX1508 output terminals (no code changes needed), see `WIRING.md`

### Problem: ESP32 resets when motors run
**Solution**: Motor current too high for USB - use external 5V 2A+ power supply, add 1000µF capacitor

### Problem: Motors jitter at center position
**Solution**: Increase `MOTOR_DEADZONE` in code (currently 200), add power filtering, check joystick calibration

### Problem: Motors twitch on boot/reset
**Solution**: Add motor enable circuit using 2N2222 transistor on GPIO5. This cuts MX1508 ground connection until code explicitly enables it. Pull-down resistors alone don't work because bootloader can drive GPIO pins. See `WIRING.md` Step 4 for circuit diagram.

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
