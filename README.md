# ESP32-C3 Wireless Joystick Controller

Wireless dual-joystick controller with servo and DC motor control using ESP32-C3 boards.

## Features

- **Dual joystick input** with button support
- **Servo control** (Joy1 X-axis → 0-180°)
- **Dual DC motor control** (Joy2 X/Y axes)
- **Two communication modes:** BLE (default) or WiFi
- **OLED displays** on both boards
- **Boot-safe motor enable circuit** (prevents motor twitch on power-up)
- **LiPo battery support** with voltage monitoring and low-battery protection

## Hardware

### Transmitter (TX)
- ESP32-C3 Super Mini
- 2x HW-504 Joystick modules
- 1.9" OLED 128x64 (SH1106, I2C)

### Receiver (RX)
- ESP32-C3 Super Mini
- 1.3" OLED 128x64 (SSD1306, I2C)
- SG90 Servo
- MX1508 Motor Driver
- 2x DC Motors (3.7V)
- 2N2222 NPN Transistor + 1K Resistor (motor enable circuit)

## Pin Assignments

### Transmitter
| GPIO | Function |
|------|----------|
| 0 | Joystick 1 X |
| 1 | Joystick 1 Y |
| 2 | Joystick 1 Button |
| 3 | Joystick 2 X |
| 4 | Joystick 2 Y |
| 5 | Joystick 2 Button |
| 6 | I2C SDA (Display) |
| 7 | I2C SCL (Display) |
| 8 | LED (blinks 1s) |
| 10 | Battery voltage sense |

### Receiver
| GPIO | Function |
|------|----------|
| 0 | Motor 1 IN1 |
| 1 | Motor 1 IN2 |
| 2 | Motor 2 IN3 |
| 3 | Motor 2 IN4 |
| 4 | Servo Signal |
| 5 | Motor Enable (transistor) |
| 6 | I2C SDA (Display) |
| 7 | I2C SCL (Display) |
| 8 | LED (blinks 3s) |
| 10 | Battery voltage sense |

## Quick Start

```bash
# Install dependencies
make install-libs

# Compile and upload both boards
make all

# Or upload individually
make upload-transmitter
make upload-receiver
```

## Control Mapping

- **Joystick 1 X-axis** → Servo angle (0-180°)
- **Joystick 2 X-axis** → Motor 1 (left/right)
- **Joystick 2 Y-axis** → Motor 2 (forward/backward)

## Motor Enable Circuit

Prevents motor twitch during boot. Required wiring:

```
GPIO5 ──[1K]── 2N2222 Base
               2N2222 Emitter ── ESP32 GND
               2N2222 Collector ── MX1508 GND
```

See `WIRING.md` for complete wiring diagrams.

## Communication Modes

**BLE Mode (Default):**
- Range: 10-30m
- No network required
- Just power on both boards

**WiFi Mode (Optional):**
1. Uncomment `#define USE_WIFI` in both .ino files
2. Configure network in code (default: "amplifi")
3. `make wifi-all`

See `WIFI_SETUP.md` for details.

## Power Requirements

**USB Power:**
- **TX:** USB power (500mA)
- **RX:** 5V 2A+ adapter recommended (motors draw significant current)

**LiPo Battery (portable):**
- **TX:** 1S LiPo 500-1000mAh (~2-4 hours runtime)
- **RX:** 1S LiPo 1000-2000mAh + MT3608 boost converter for 5V

See `WIRING.md` for battery circuit diagrams.

## Documentation

- `WIRING.md` - Complete wiring guide with diagrams
- `WIFI_SETUP.md` - WiFi mode configuration
- `HARDWARE_FILTERING.md` - ADC noise filtering
- `CLAUDE.md` - Technical reference

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Motors twitch on boot | Add motor enable circuit (see WIRING.md) |
| Display blank | Check 3.3V power, I2C wiring |
| No BLE connection | Reset both boards, wait 10s |
| Motors weak | Use 5V 2A+ power adapter |
| ESP32 resets | Insufficient power for motors |
| Battery icon shows empty | Check voltage divider wiring (GPIO10) |
| "LOW BATTERY" warning | Charge or replace LiPo battery |
| Motors disabled | Battery voltage below 3.3V, charge battery |

## Make Commands

```bash
make all                 # Compile + upload both (BLE)
make wifi-all            # Compile + upload both (WiFi)
make compile             # Compile only
make upload-transmitter  # Upload TX
make upload-receiver     # Upload RX
make list-ports          # Show connected boards
make monitor-transmitter # Serial monitor TX
make install-libs        # Install libraries
```

## Project Structure

```
├── transmitter/transmitter.ino  # TX code
├── receiver/receiver.ino        # RX code
├── Makefile                     # Build system
├── WIRING.md                    # Wiring guide
├── WIFI_SETUP.md                # WiFi configuration
├── HARDWARE_FILTERING.md        # ADC filtering
└── CLAUDE.md                    # Technical reference
```
