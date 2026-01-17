# Servo Setup Guide - SG90 on Receiver

## Overview

The receiver board now controls a servo motor based on Joystick 1 X-axis movement from the transmitter.

**Configuration:**
- **Servo Model:** SG90 (mini servo)
- **Control Input:** Joystick 1 X-axis (from transmitter)
- **Servo Angle Range:** 0° to 180°
- **Control Pin:** GPIO0 (receiver board)

---

## Wiring Diagram

### SG90 Servo Pinout
```
Looking at servo with wires facing you:

    [Servo Body]
    ||||
    ||||  (3 wires)
    ||||
    ||||
   Brown  Red  Orange
    |     |     |
   GND   VCC   Signal
```

**Wire Colors (Standard SG90):**
- **Brown** = Ground (GND)
- **Red** = Power (VCC) - 5V
- **Orange/Yellow** = Signal (PWM control)

### Connection to ESP32-C3 Receiver

```
SG90 Servo                ESP32-C3 Receiver
──────────────────────────────────────────────
Brown (GND)      ────────> GND
Red (VCC)        ────────> 5V
Orange (Signal)  ────────> GPIO0
```

---

## Important Power Notes

### Servo Power Requirements

⚠️ **CRITICAL:** SG90 servo operates at **4.8V - 6V** (typically 5V)

**Power Options:**

### Option 1: Low-Load Operation (Single Servo, No Load)
```
Connect servo VCC (red) to ESP32 5V pin
```
- ✓ Simple, no external power needed
- ✓ Works for testing/light loads
- ⚠️ ESP32 5V pin is USB passthrough (500mA limit)
- ⚠️ May cause brownouts if servo under heavy load
- ⚠️ May reset ESP32 if servo stalls

### Option 2: External Power (RECOMMENDED for reliable operation)
```
                     ┌──────────────┐
External 5V ─────────┤ VCC   Servo  │
Power Supply         │              │
         │           │ Signal       ├──> GPIO0 (ESP32)
         │           │              │
        GND ─────────┤ GND          │
         │           └──────────────┘
         │
         └────────────> ESP32 GND (common ground!)
```

**External Power Source Options:**
- USB power bank (5V output)
- 5V wall adapter
- 4x AA batteries (6V, use with caution)
- Separate 5V regulator from higher voltage

**IMPORTANT:** Always connect grounds together (ESP32 GND ↔ External Power GND)

---

## Current Receiver Board Connections

```
                        ┌─────────────────┐
                        │  ESP32-C3       │
   OLED Display         │  Receiver       │
 (GND,VCC,SCK,SDA)      │                 │
   ┌──────────┐     ┌───┤ GPIO6 (SDA)     │
   │  GND  ───┼─────┼───┤ GND             │
   │  VCC  ───┼─────┼───┤ 3.3V            │
   │  SCK  ───┼─────────┤ GPIO7 (SCL)     │
   │  SDA  ───┼─────────┤ GPIO6           │
   └──────────┘         │                 │
                        │                 │
   SG90 Servo           │                 │
 (Brown,Red,Orange)     │                 │
   ┌──────────┐         │                 │
   │ Brown ───┼─────────┤ GND             │
   │ Red   ───┼─────────┤ 5V              │
   │ Orange───┼─────────┤ GPIO0 (PWM)     │
   └──────────┘         │                 │
                        │                 │
                        │  USB-C          │◄──── Power from Computer
                        └─────────────────┘
```

---

## Software Configuration

### Current Mapping
- **Input:** Joystick 1 X-axis value (0 to 4095)
- **Output:** Servo angle (0° to 180°)
- **Mapping:** Linear mapping
  ```
  Joy1 X = 0    → Servo = 0°   (Full Left)
  Joy1 X = 2048 → Servo = 90°  (Center)
  Joy1 X = 4095 → Servo = 180° (Full Right)
  ```

### Display Feedback
Receiver OLED now shows:
```
J1: XXXX,YYYY
J2: XXXX,YYYY
Servo: XXX°
```

---

## Changing Configuration

### To Use Different Joystick/Axis

In `receiver/receiver.ino`, find this line (~line 247):
```c
int servoAngle = map(joystickData.joy1_x, 0, 4095, 0, 180);
```

**Change to:**
- **Joy1 Y-axis:** `joystickData.joy1_y`
- **Joy2 X-axis:** `joystickData.joy2_x`
- **Joy2 Y-axis:** `joystickData.joy2_y`

### To Change Servo Angle Range

Change the mapping:
```c
// Example: Limit servo to 45° - 135° (quarter rotation each side)
int servoAngle = map(joystickData.joy1_x, 0, 4095, 45, 135);
```

### To Reverse Servo Direction

Swap the angle values:
```c
// Reversed: Left joystick = 180°, Right joystick = 0°
int servoAngle = map(joystickData.joy1_x, 0, 4095, 180, 0);
```

### To Change Control Pin

In `receiver/receiver.ino`, change:
```c
#define SERVO_PIN 0  // Change to any available GPIO
```

**Available GPIO pins on receiver:**
- GPIO0, GPIO1, GPIO2, GPIO3, GPIO4, GPIO5 (currently unused)
- Avoid GPIO6, GPIO7 (used by OLED I2C)
- Avoid GPIO8 (LED on some boards)

---

## Installation & Upload

### Step 1: Install Servo Library
```bash
make install-libs
```

This will install:
- Adafruit GFX Library
- Adafruit SSD1306
- ESP32Servo (NEW)

### Step 2: Compile and Upload
```bash
make all
```

Or upload only receiver:
```bash
make upload-receiver
```

---

## Testing Procedure

1. **Wire the servo** according to the diagram above
2. **Upload the code** to receiver board
3. **Power on transmitter** (with joysticks)
4. **Power on receiver** (with servo and OLED)
5. **Wait for BLE connection** (~10 seconds)
6. **Move Joystick 1 left/right** (X-axis)
7. **Observe:**
   - Servo should move 0° to 180°
   - Display shows "Servo: XX°"
   - Should follow joystick smoothly

---

## Troubleshooting

### Servo Not Moving
1. Check wiring (especially signal wire to GPIO0)
2. Verify servo has power (red wire to 5V)
3. Check serial monitor: "Servo initialized at 90 degrees"
4. Try manual test code (see below)

### Servo Jitters/Twitches
1. **Noise on signal line** - Add 0.1µF capacitor between GPIO0 and GND
2. **Power supply noise** - Add 100µF capacitor between servo VCC and GND
3. **Insufficient power** - Use external 5V power supply
4. **Software filtering** - Already implemented in transmitter

### ESP32 Resets When Servo Moves
1. **Insufficient power** - Servo drawing too much current
2. **Solution:** Use external 5V power supply for servo
3. **Brownout detector** - Triggering due to voltage drop

### Servo Moves Erratically
1. Check BLE connection is stable
2. Verify joystick values are stable (check transmitter display)
3. Increase software filtering (SMOOTHING_ALPHA in transmitter code)

### Servo Doesn't Reach Full Range
1. SG90 servos vary - some don't reach exactly 0° or 180°
2. Adjust mapping: `map(joystickData.joy1_x, 0, 4095, 5, 175)`
3. Some servos are limited to ~170° rotation

---

## Manual Servo Test Code

If servo isn't working, test with this simple code:

```cpp
#include <ESP32Servo.h>

Servo testServo;

void setup() {
  testServo.attach(0);  // GPIO0
  testServo.write(90);  // Center
  delay(1000);
}

void loop() {
  testServo.write(0);    // 0 degrees
  delay(1000);
  testServo.write(90);   // 90 degrees
  delay(1000);
  testServo.write(180);  // 180 degrees
  delay(1000);
}
```

Upload this to receiver alone (disconnect transmitter). Servo should sweep 0° → 90° → 180° → repeat.

---

## Future Enhancements

### Add Multiple Servos
- Use GPIO1, GPIO2, GPIO3, etc. for additional servos
- Control with different joystick axes
- Example: Joy1 X → Servo1, Joy1 Y → Servo2

### Add Servo Smoothing
- Implement exponential smoothing on receiver side
- Prevents jerky servo movements
- Better response than joystick filtering alone

### Add Deadband
- Create neutral zone where servo doesn't move
- Useful if joystick center isn't exactly centered
- Example: Don't move servo if joystick within ±200 of center

---

## SG90 Servo Specifications

- **Operating Voltage:** 4.8V - 6V
- **Operating Current:** ~100mA (no load), ~250mA (typical), ~650mA (stall)
- **Stall Torque:** 1.8 kg·cm (4.8V)
- **Operating Speed:** 0.1 sec/60° (4.8V)
- **Rotation:** ~180° (varies by unit)
- **Control Signal:** PWM, 50Hz (20ms period)
  - 1ms pulse = 0°
  - 1.5ms pulse = 90°
  - 2ms pulse = 180°
- **Weight:** 9g
- **Dimensions:** 22.2 × 11.8 × 31mm

---

*Servo control is now active on the receiver board!*
