# ESP32-C3 Dual Joystick BLE Communication System

This project implements Bluetooth Low Energy (BLE) communication between two ESP32-C3 Super Mini boards. One board reads two joystick modules and transmits the data to the second board, with both displaying the joystick positions on OLED displays.

## Hardware Requirements

- 2x ESP32-C3 Super Mini boards
- 2x 0.91" OLED Display (SSD1306, I2C)
- 2x HW-504 Joystick Module
- Breadboards and jumper wires
- USB-C cables for programming

## Features

- Dual joystick input with button support
- Real-time BLE data transmission
- OLED display on both boards showing joystick directions
- LED blinking indicator on transmitter
- Automatic reconnection handling

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

## Compilation

### Compile Both Projects
```bash
make compile
```

### Compile Individual Projects
```bash
make compile-transmitter
make compile-receiver
```

## Uploading to Boards

### 1. Find Serial Ports
```bash
make list-ports
```

This will show available ports, typically:
- macOS: `/dev/cu.usbserial-XXXX` or `/dev/cu.wchusbserial-XXXX`
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- Windows: `COM3`, `COM4`, etc.

### 2. Upload Transmitter
```bash
make upload-transmitter TX_PORT=/dev/cu.usbserial-1234
```

### 3. Upload Receiver
```bash
make upload-receiver RX_PORT=/dev/cu.usbserial-5678
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

1. Power on both ESP32 boards
2. The transmitter will start advertising as "ESP32_Joystick_TX"
3. The receiver will scan and automatically connect
4. Move the joysticks - both displays will show:
   - Connection status
   - Joystick 1 direction (UP/DOWN/LEFT/RIGHT/CENTER)
   - Joystick 2 direction (UP/DOWN/LEFT/RIGHT/CENTER)
   - Button press status
5. The transmitter's built-in LED will blink every 500ms

## Display Output Format

```
TX: Connected       (or "TX: Waiting..." / "RX: Connected" / "RX: Scanning...")
J1: RIGHT
J2: UP
J1: PRESSED         (only shown when button pressed)
J2: PRESSED         (only shown when button pressed)
```

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
├── Makefile                  # Build automation
├── README.md                 # This file
├── transmitter/
│   └── transmitter.ino       # Transmitter code
└── receiver/
    └── receiver.ino          # Receiver code
```

## Technical Details

### BLE Configuration
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Update Rate: 100ms (10Hz)
- Data Structure: 12 bytes (6x int16_t/uint8_t)

### Joystick Calibration
- Center value: ~2048 (12-bit ADC)
- Threshold: 1000 units
- Adjust in `getDirection()` function if needed

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
int threshold = 1000;  // Increase for less sensitivity
```

## License

This project is provided as-is for educational purposes.

## Credits

Built with:
- Arduino IDE/CLI
- ESP32 Arduino Core
- Adafruit GFX & SSD1306 libraries
