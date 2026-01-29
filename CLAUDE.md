# ESP32 Wireless Joystick Controller - Technical Reference

**Last Updated:** 2026-01-27

## Project Summary

Wireless dual-joystick controller with servo and DC motor control between two ESP32-C3 boards.

**Control Mapping:**
- Joy1 X-axis → Servo (0-180°)
- Joy2 X-axis → Motor 1
- Joy2 Y-axis → Motor 2

**Communication:** BLE (default) or WiFi (optional)

---

## Hardware Configuration

### Transmitter (TX)
| GPIO | Function | Notes |
|------|----------|-------|
| 0 | Joy1 VRx | ADC |
| 1 | Joy1 VRy | ADC |
| 2 | Joy1 SW | Button (active LOW) |
| 3 | Joy2 VRx | ADC |
| 4 | Joy2 VRy | ADC |
| 5 | Joy2 SW | Button (active LOW) |
| 6 | I2C SDA | Display |
| 7 | I2C SCL | Display |
| 8 | LED | Blinks every 1s |
| 10 | VBAT | Battery voltage (via divider) |

- **Display:** 1.9" OLED 128x64 SH1106 @ 0x3C
- **Library:** Adafruit_SH110X

### Receiver (RX)
| GPIO | Function | Notes |
|------|----------|-------|
| 0 | Motor1 IN1 | MX1508 forward |
| 1 | Motor1 IN2 | MX1508 reverse |
| 2 | Motor2 IN3 | MX1508 forward |
| 3 | Motor2 IN4 | MX1508 reverse |
| 4 | Servo | PWM signal (SG90 or GS-1502) |
| 5 | Motor Enable | 2N2222 transistor base |
| 6 | I2C SDA | Display |
| 7 | I2C SCL | Display |
| 8 | LED | Blinks every 3s |
| 10 | VBAT | Battery voltage (via divider) |

- **Display:** 1.3" OLED 128x64 SSD1306 @ 0x3C
- **Library:** Adafruit_SSD1306
- **Motor PWM:** 20kHz, 8-bit resolution

---

## Motor Enable Circuit

**Purpose:** Prevents motor twitch during boot/reset. The ESP32 bootloader can drive GPIO pins briefly, causing unintended motor movement.

**Solution:** Transistor controls MX1508 ground connection. Motors only powered after code sets GPIO5 HIGH.

```
GPIO5 ──[1K]──┬── 2N2222 Base (B)
              │
MX1508 GND ───┴── 2N2222 Collector (C)
              │
ESP32 GND ────┴── 2N2222 Emitter (E)
```

**Code behavior:**
- Boot: GPIO5 LOW → transistor OFF → MX1508 has no GND → motors can't run
- After setup(): GPIO5 HIGH → transistor ON → normal operation

---

## Joystick Calibration

Measured center values (at rest):

| Joystick | X Center | Y Center |
|----------|----------|----------|
| J1 | 2235 | 2217 |
| J2 | 2215 | 2255 |

**Note:** Values are device-specific. Measure yours if behavior seems off.

---

## Communication

### BLE Mode (Default)
- Service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Characteristic UUID: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Update rate: 10Hz

### WiFi Mode (Optional)
- Protocol: UDP port 4210
- Network: "amplifi" / "123qwe123"
- Discovery: mDNS (esp32-joystick-tx.local)

Enable WiFi: Uncomment `#define USE_WIFI` in both .ino files.

---

## Data Structure

```c
struct JoystickData {
  int16_t joy1_x;    // 0-4095
  int16_t joy1_y;    // 0-4095
  uint8_t joy1_sw;   // LOW=pressed
  int16_t joy2_x;    // 0-4095
  int16_t joy2_y;    // 0-4095
  uint8_t joy2_sw;   // LOW=pressed
}; // 12 bytes
```

---

## Power Requirements

| Component | Voltage | Current |
|-----------|---------|---------|
| ESP32-C3 | 3.3V | 200mA |
| Display | 3.3V | 20mA |
| Servo (SG90) | 4.8-6V | 100-500mA |
| Servo (GS-1502) | 3.7-5V | 50-150mA |
| Motors (x2) | 5V via MX1508 | 1-2A each |

**TX:** USB power sufficient, or 1S LiPo (500-1000mAh)
**RX:** 5V 2A+ adapter recommended, or 1S LiPo (GS-1502 needs no boost, SG90 needs MT3608)

---

## Battery Monitoring

Both boards support LiPo battery operation with voltage monitoring.

**Circuit:** Voltage divider (10K + 10K) on GPIO10
- Divider ratio: 0.5 (4.2V → 2.1V safe for ADC)
- Code multiplies by 2.0 to get actual voltage

**Thresholds:**
| Voltage | Status | Action |
|---------|--------|--------|
| 4.2V | Full | Normal |
| 3.5V | Low | Warning icon flashes |
| 3.3V | Critical | Motors disabled (RX) |
| 3.2V | Empty | Boot halted |

**Code pattern:**
```c
float readBatteryVoltage() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(VBAT_PIN);
  }
  float avgRaw = sum / 10;
  return (avgRaw / 4095.0) * 3.3 * VBAT_DIVIDER;
}
```

---

## Build Commands

```bash
make all                 # BLE mode
make wifi-all            # WiFi mode
make upload-receiver     # Upload RX only
make upload-transmitter  # Upload TX only
make list-ports          # Show USB ports
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Motors twitch on boot | Add motor enable circuit (2N2222 on GPIO5) |
| Servo not responding | Check `outputsEnabled` flag, verify BLE connection |
| Display dim/blank | Verify 3.3V power, check I2C wiring |
| ESP32 resets | Insufficient power, use 5V 2A+ adapter |
| Motors weak | Power supply can't provide enough current |
| Joystick center off | Update center values in code |
| Battery icon empty | Check voltage divider (2x 10K on GPIO10) |
| "LOW BATTERY" shown | Charge LiPo, voltage below 3.3V |
| Motors disabled | Battery critically low, charge immediately |

---

## File Structure

```
transmitter/transmitter.ino  # TX code (joysticks + display)
receiver/receiver.ino        # RX code (servo + motors + display)
Makefile                     # Build automation
WIRING.md                    # Complete wiring guide
WIFI_SETUP.md                # WiFi configuration
HARDWARE_FILTERING.md        # ADC noise filtering
```

---

## Key Code Patterns

**Non-blocking timing:**
```c
static unsigned long lastUpdate = 0;
if (millis() - lastUpdate > 100) {
  // Do work every 100ms
  lastUpdate = millis();
}
```

**ADC filtering:**
```c
// Oversampling (4 samples)
int readADC(int pin) {
  long sum = 0;
  for (int i = 0; i < 4; i++) {
    sum += analogRead(pin);
  }
  return sum / 4;
}

// Exponential moving average
filtered = 0.3 * raw + 0.7 * filtered;
```

**Motor control:**
```c
if (motor_speed > 0) {
  ledcWrite(IN1, motor_speed);  // Forward
  ledcWrite(IN2, 0);
} else {
  ledcWrite(IN1, 0);
  ledcWrite(IN2, -motor_speed); // Reverse
}
```
