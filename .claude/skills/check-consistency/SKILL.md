---
name: check-consistency
description: Verify configuration values are consistent across all ESP32 code files and documentation
allowed-tools: Read, Grep, Glob
---

# Configuration Consistency Check

Verify that configuration values are consistent across all files and documentation.

## Items to Cross-Check

### 1. Pin Assignments
Compare pin definitions in:
- transmitter/transmitter.ino
- receiver/receiver.ino
- receiver_compact/receiver_compact.ino
- CLAUDE.md
- WIRING.md

Verify GPIO numbers match documentation.

### 2. Communication Parameters
- BLE Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- BLE Characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- WiFi UDP port: 4210
- WiFi network: "amplifi"
- mDNS name: "esp32-joystick-tx.local"

### 3. Data Structure
- JoystickData struct: 12 bytes
- Field order and types match TX/RX

### 4. Joystick Calibration
- J1 X center: ~2235
- J1 Y center: ~2217
- J2 X center: ~2215
- J2 Y center: ~2255
- Deadzone values

### 5. Battery Thresholds
| Voltage | Meaning |
|---------|---------|
| 4.2V | Full |
| 3.5V | Low warning |
| 3.3V | Critical (disable motors) |
| 3.2V | Empty (halt boot) |

### 6. Timing Constants
- Display update rate
- LED blink intervals (TX: 1s, RX: 3s)
- BLE notification rate
- Battery check interval

### 7. Motor/Servo Parameters
- PWM frequency: 20kHz for motors
- PWM resolution: 8-bit
- Servo range: 0-180 degrees
- Motor deadzone

## Output Format

```
=== CONSISTENCY CHECK ===

MATCHES:
- [item] Consistent across all files

MISMATCHES:
- [item] File1: value1, File2: value2

DOCUMENTATION DRIFT:
- [item] Code says X, CLAUDE.md says Y

RECOMMENDATIONS:
- Extract to shared header file
- Update documentation
```

Read all relevant files and produce a consistency report.
