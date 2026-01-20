# ESP32-C3 Dual Joystick Wireless Communication System

This project implements wireless communication between two ESP32-C3 Super Mini boards with **two communication modes**:
- **BLE Mode (Default):** Bluetooth Low Energy for simple setup and low power
- **WiFi Mode:** UDP over WiFi for longer range and lower latency

One board reads two joystick modules and transmits the data to the second board, with both displaying the joystick positions on OLED displays and controlling a servo motor.

## Hardware Requirements

- 2x ESP32-C3 Super Mini boards
- 2x 0.91" OLED Display (SSD1306, I2C)
- 2x HW-504 Joystick Module
- Breadboards and jumper wires
- USB-C cables for programming

## Features

- **Dual Communication Modes:**
  - BLE mode for simple setup and low power (default)
  - WiFi mode for longer range (50-100m) and lower latency
  - Compile-time switching (no hardware changes required)
- Dual joystick input with button support
- Real-time wireless data transmission @ 10Hz
- OLED displays with visual joystick position indicators
- Servo motor control based on joystick input
- Software filtering for smooth, stable readings
- LED blinking indicator on transmitter
- Automatic reconnection/discovery handling
- mDNS discovery in WiFi mode

## Wiring Diagrams

### Transmitter Board (with 2 Joysticks + OLED)

```
ESP32-C3 Super Mini (Transmitter)
====================================

OLED Display (0.91" SSD1306) - Pin order: GND, VCC, SCK, SDA
  GND  -> GND
  VCC  -> 3.3V
  SCK  -> GPIO7 (I2C Clock)
  SDA  -> GPIO6 (I2C Data)

Joystick 1 (HW-504) - Pin order: GND, 5V, VRX, VRY, SW
  GND  -> GND
  5V   -> 5V (or 3.3V)
  VRX  -> GPIO0 (X-axis analog)
  VRY  -> GPIO1 (Y-axis analog)
  SW   -> GPIO2 (Button, active LOW)

Joystick 2 (HW-504) - Pin order: GND, 5V, VRX, VRY, SW
  GND  -> GND
  5V   -> 5V (or 3.3V)
  VRX  -> GPIO3 (X-axis analog)
  VRY  -> GPIO4 (Y-axis analog)
  SW   -> GPIO5 (Button, active LOW)

Built-in LED:
  GPIO8 (automatically blinks)
```

### Receiver Board (OLED only)

```
ESP32-C3 Super Mini (Receiver)
====================================

OLED Display (0.91" SSD1306) - Pin order: GND, VCC, SCK, SDA
  GND  -> GND
  VCC  -> 3.3V
  SCK  -> GPIO7 (I2C Clock)
  SDA  -> GPIO6 (I2C Data)
```

## Pin Summary

### Transmitter Pins:
| Component | Pin | GPIO | Description |
|-----------|-----|------|-------------|
| OLED SDA | SDA | 6 | I2C Data |
| OLED SCL | SCL | 7 | I2C Clock |
| Joy1 VRx | A0 | 0 | Joystick 1 X-axis |
| Joy1 VRy | A1 | 1 | Joystick 1 Y-axis |
| Joy1 SW | D2 | 2 | Joystick 1 Button |
| Joy2 VRx | A3 | 3 | Joystick 2 X-axis |
| Joy2 VRy | A4 | 4 | Joystick 2 Y-axis |
| Joy2 SW | D5 | 5 | Joystick 2 Button |
| LED | D8 | 8 | Built-in LED |

### Receiver Pins:
| Component | Pin | GPIO | Description |
|-----------|-----|------|-------------|
| OLED SDA | SDA | 6 | I2C Data |
| OLED SCL | SCL | 7 | I2C Clock |

## Software Setup

### 1. Install Arduino CLI

On macOS:
```bash
make install-cli
```

Or manually:
```bash
brew install arduino-cli
```

### 2. Install ESP32 Core

```bash
make install-core
```

This will:
- Initialize Arduino CLI configuration
- Add ESP32 board manager URL
- Install ESP32 board support

### 3. Install Required Libraries

```bash
make install-libs
```

This installs:
- Adafruit GFX Library
- Adafruit SSD1306
- ESP32Servo

### 4. Choose Communication Mode

The project supports two communication modes. **BLE mode is enabled by default** and requires no changes.

#### Option A: BLE Mode (Default)
- No configuration needed
- Ready to compile and upload
- Range: 10-30m
- Lower power consumption

#### Option B: WiFi Mode (Optional)
To enable WiFi mode:

1. **Edit both files** (`transmitter/transmitter.ino` and `receiver/receiver.ino`):
   ```cpp
   // Uncomment this line (around line 5):
   #define USE_WIFI
   ```

2. **Configure WiFi network** (if different from default):
   ```cpp
   #define WIFI_SSID "your-network-name"
   #define WIFI_PASSWORD "your-password"
   ```

3. **Compile and upload** using WiFi targets (see below)

**WiFi Mode Benefits:**
- Longer range: 50-100m
- Lower latency: ~50-100ms
- Works with existing WiFi network

**See WIFI_SETUP.md for detailed WiFi configuration guide.**

## Compilation

### BLE Mode (Default)

Compile both projects:
```bash
make compile
```

Compile individual projects:
```bash
make compile-transmitter
make compile-receiver
```

### WiFi Mode

Compile both projects (WiFi mode):
```bash
make wifi-compile
```

Compile individual projects (WiFi mode):
```bash
make wifi-compile-transmitter
make wifi-compile-receiver
```

**Note:** WiFi mode requires `#define USE_WIFI` uncommented in both `.ino` files.

## Uploading to Boards

### 1. Find Serial Ports
```bash
make list-ports
```

This will show available ports, typically:
- macOS: `/dev/cu.usbserial-XXXX` or `/dev/cu.wchusbserial-XXXX`
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- Windows: `COM3`, `COM4`, etc.

### 2. Upload Both Boards

**BLE Mode:**
```bash
make all  # Compiles and uploads both boards
```

Or individually:
```bash
make upload-transmitter TX_PORT=/dev/cu.usbserial-1234
make upload-receiver RX_PORT=/dev/cu.usbserial-5678
```

**WiFi Mode:**
```bash
make wifi-all  # Compiles and uploads both boards (WiFi mode)
```

Or individually:
```bash
make wifi-upload-transmitter TX_PORT=/dev/cu.usbserial-1234
make wifi-upload-receiver RX_PORT=/dev/cu.usbserial-5678
```

## Serial Monitor

### Monitor Transmitter
```bash
make monitor-transmitter TX_PORT=/dev/cu.usbserial-1234
```

### Monitor Receiver
```bash
make monitor-receiver RX_PORT=/dev/cu.usbserial-5678
```

Press `Ctrl+C` to exit the monitor.

## Usage

### BLE Mode
1. Power on both ESP32 boards
2. The transmitter will start advertising as "ESP32_Joystick_TX"
3. The receiver will scan and automatically connect
4. Move the joysticks - both displays will show:
   - Visual joystick position circles
   - Connection status ("TX" or "RX")
   - Joystick raw values
   - Servo position (receiver only)
5. The transmitter's built-in LED will blink every 500ms
6. Servo responds to Joystick 1 X-axis movement (0-180°)

### WiFi Mode
1. Power on both ESP32 boards
2. Both boards connect to configured WiFi network ("amplifi" by default)
3. Receiver discovers transmitter via mDNS (esp32-joystick-tx.local)
4. Move the joysticks - both displays will show:
   - Visual joystick position circles
   - Connection status ("WiFi")
   - Joystick raw values
   - Servo position (receiver only)
5. The transmitter's built-in LED will blink every 500ms
6. Servo responds to Joystick 1 X-axis movement (0-180°)

**Range:** BLE ~10-30m, WiFi ~50-100m

## Display Output

### Transmitter Display
```
┌──────────────────────────────────────┐
│ J1    ○       J2    ○        WiFi/TX │  ← Joystick circles + status
│       /│\            /│\              │
│      ┼─○            ┼─○               │  ← Crosshairs show zero position
│                                       │
│ 3515               3352               │  ← Raw X values
└──────────────────────────────────────┘
```

### Receiver Display
```
┌──────────────────────────────────────┐
│ J1  ○     J2  ○              WiFi/RX │  ← Joystick circles + status
│     /│\        /│\                    │
│    ┼─○        ┼─○                     │
│ S:90°                                 │  ← Servo angle
│ ▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░ │  ← Servo position bar
└──────────────────────────────────────┘
```

**Status Indicators:**
- BLE mode: "TX" / "RX" (top right)
- WiFi mode: "WiFi" (top right)
- Button press: Filled circle (hollow when released)

## Troubleshooting

### OLED Not Working
- Check I2C address (default 0x3C)
- Verify SDA/SCL connections
- Try swapping SDA/SCL if display doesn't initialize

### Joysticks Not Responding
- Verify analog pins (GPIO0,1,3,4 support ADC on ESP32-C3)
- Check power supply (3.3V or 5V)
- Test with serial monitor to see raw values

### BLE Connection Issues
- Keep boards within 10 meters
- Reset both boards
- Check serial monitor for connection logs
- Ensure only one receiver is trying to connect

### WiFi Connection Issues
- Verify SSID and password in code match your network
- Ensure 2.4GHz WiFi is enabled (ESP32-C3 doesn't support 5GHz)
- Check router allows mDNS/Bonjour (or receiver will use broadcast fallback)
- Verify both boards get IP addresses (check serial monitor)
- Ensure UDP port 4210 is not blocked by firewall
- See `WIFI_SETUP.md` for detailed WiFi troubleshooting

### Compilation Errors
- Ensure ESP32 core is installed: `make install-core`
- Verify libraries are installed: `make install-libs`
- Update core: `arduino-cli core update-index && arduino-cli core upgrade`

### Upload Fails
- Hold BOOT button while uploading
- Check correct port is selected
- Verify USB cable supports data (not charge-only)

## Project Structure

```
.
├── Makefile                  # Build automation (BLE + WiFi targets)
├── README.md                 # This file
├── WIFI_SETUP.md             # WiFi configuration guide
├── CLAUDE.md                 # AI catch-up guide
├── SERVO_SETUP.md            # Servo configuration
├── HARDWARE_FILTERING.md     # ADC filtering guide
├── transmitter/
│   └── transmitter.ino       # Transmitter code (BLE + WiFi)
└── receiver/
    └── receiver.ino          # Receiver code (BLE + WiFi + servo)
```

## Technical Details

### Communication Modes

**BLE Configuration:**
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Update Rate: 100ms (10Hz)
- Range: 10-30m
- Power: Low

**WiFi Configuration:**
- Protocol: UDP
- Port: 4210
- Network: "amplifi" (SSID), "123qwe123" (password)
- Discovery: mDNS (esp32-joystick-tx.local)
- Update Rate: 100ms (10Hz)
- Range: 50-100m
- Power: Higher

**Data Structure (Both Modes):**
- 12 bytes: 2x joysticks, each with X/Y (int16_t) + button (uint8_t)

### Joystick Calibration
- Center values (measured): J1(3515, 3234), J2(3352, 3510)
- ADC resolution: 12-bit (0-4095)
- Software filtering: Oversampling (4x) + exponential moving average (α=0.3)
- Threshold: 1500 units for direction detection

### Servo Control
- GPIO: 0 (receiver)
- Control source: Joystick 1 X-axis
- Range: 0-180°
- Library: ESP32Servo

### LED Blink Rate
- 500ms on/off cycle (1Hz)
- Only on transmitter board

## Customization

### Change Update Rate
Edit both sketches, modify:
```cpp
if (millis() - lastUpdate > 100) {  // Change 100 to desired ms
```

### Change LED Blink Rate
Edit transmitter.ino:
```cpp
if (millis() - lastBlink > 500) {  // Change 500 to desired ms
```

### Change Direction Threshold
Edit `getDirection()` function:
```cpp
int threshold = 1500;  // Increase for less sensitivity
```

### Switch Between BLE and WiFi
1. Edit both `.ino` files
2. Uncomment `#define USE_WIFI` for WiFi mode
3. Comment it out for BLE mode
4. Recompile and upload

See `WIFI_SETUP.md` for detailed WiFi configuration.

## License

This project is provided as-is for educational purposes.

## Credits

Built with:
- Arduino IDE/CLI
- ESP32 Arduino Core
- Adafruit GFX & SSD1306 libraries
- ESP32Servo library
- ESP32 WiFi & mDNS libraries
