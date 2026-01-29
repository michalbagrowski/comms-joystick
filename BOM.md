# Bill of Materials (BOM)

## Quick Summary

| Build | Components | Est. Cost |
|-------|------------|-----------|
| Standard TX + RX | Full-size modules | ~$50 |
| Standard TX + Compact RX | Mixed | ~$34 |
| Compact RX only | Miniaturized | ~$20 |

---

## Standard Build (Full Size)

### Transmitter (TX) - ESP32-C3 Super Mini

| Qty | Component | Specs | Est. Price | Notes |
|-----|-----------|-------|------------|-------|
| 1 | ESP32-C3 Super Mini | 22x18mm | $4 | AliExpress |
| 2 | HW-504 Joystick Module | Analog XY + button | $2 | |
| 1 | 1.9" OLED Display | 128x64, I2C, SH1106 | $4 | 3.3V only! |
| 1 | USB-C Cable | Data + power | $2 | |
| - | Jumper wires | Various | $2 | |

**TX Subtotal: ~$14**

### Receiver (RX) - Standard Build

| Qty | Component | Specs | Est. Price | Notes |
|-----|-----------|-------|------------|-------|
| 1 | ESP32-C3 Super Mini | 22x18mm | $4 | AliExpress |
| 1 | MX1508 Motor Driver | Dual H-bridge | $2 | |
| 1 | SG90 Servo | Standard size | $3 | |
| 2 | DC Motors | 3-6V, with gearbox | $4 | |
| 1 | 1.3" OLED Display | 128x64, I2C, SSD1306 | $3 | 3.3V only! |
| 1 | 2N2222 NPN Transistor | Motor enable circuit | $0.20 | Prevents boot twitch |
| 1 | 1K Resistor | 1/4W | $0.05 | Base resistor for transistor |
| 1 | 5V 2A Power Adapter | USB-C or barrel | $5 | For motors |

**RX Standard Subtotal: ~$21**

### Optional - Standard Build

| Qty | Component | Purpose | Est. Price |
|-----|-----------|---------|------------|
| 1 | 1000uF Electrolytic Cap | Motor power smoothing | $0.50 |
| 2 | 100uF Electrolytic Cap | Power stability | $0.30 |
| 4 | 0.1uF Ceramic Cap | ADC filtering | $0.20 |
| 2 | Breadboard 830pt | Prototyping | $6 |

---

## Compact Build (Miniaturized RX)

### Transmitter (TX) - Same as Standard

Use standard TX build above (~$14)

### Receiver (RX) - Compact Build

| Qty | Component | Size | Est. Price | Notes |
|-----|-----------|------|------------|-------|
| 1 | **Seeed XIAO ESP32-C3** | 21x17mm | $5 | Built-in LiPo charging! |
| 1 | **DRV8833 Breakout** | 10x15mm | $2 | Has nSLEEP pin |
| 2 | Micro DC Motors | N20 or smaller | $3 | |
| 1 | **GS-1502 Linear Servo** | 21x15x12mm | $3-4 | 3.7-5V, no boost needed |
| 1 | LiPo Battery | 100-200mAh, 1S | $4 | 501220 or 602025 size |
| 1 | **1S LiPo Protection PCB** | 5x10mm | $0.30 | **Required for safety** |
| 2 | 10K Resistor | 0402 or 0603 SMD | $0.10 | Voltage divider |
| 1 | 100uF Capacitor | SMD or small | $0.20 | Power smoothing |
| - | 30AWG Silicone Wire | Thin, flexible | $2 | |

**RX Compact Subtotal: ~$20**

### Optional - Compact Build

| Qty | Component | Purpose | Est. Price |
|-----|-----------|---------|------------|
| 1 | 0.42" OLED Display | Debugging only | $4 |
| 1 | MT3608 Mini Boost | 5V for servo (only if PZ-15320) | $1 |
| 1 | Slide Switch | Power on/off | $0.50 |
| 1 | JST 1.25mm Connector | Battery plug | $0.30 |

---

## LiPo Battery Protection (REQUIRED for safe operation)

The code has software low-voltage cutoff, but **hardware protection is essential** to prevent deep discharge if the ESP32 crashes or battery drains during storage.

### Option 1: 1S Protection PCB Module (Recommended)

| Qty | Component | Size | Est. Price | Notes |
|-----|-----------|------|------------|-------|
| 2 | 1S LiPo Protection Board | 5x10mm | $0.30 each | Easiest - solder inline with battery |

Search: "1S 3.7V protection PCB" or "1S BMS module"

**Protects against:**
- Over-discharge (cuts off at ~2.5V)
- Over-charge (cuts off at ~4.25V)
- Short circuit
- Over-current (~3A typical)

**Wiring:** Battery(+) -> Protection PCB -> Load

### Option 2: Protected LiPo Cell

Buy batteries with protection built-in (adds ~2mm to length, +$1):
- Search: "protected 501220 LiPo" or "protected 602025 LiPo"
- Small PCB already attached to cell
- No extra wiring needed

### Option 3: DIY (Smallest footprint)

| Qty | Component | Package | Est. Price | Notes |
|-----|-----------|---------|------------|-------|
| 2 | DW01A Protection IC | SOT-23-6 | $0.10 | Protection logic |
| 2 | FS8205A Dual MOSFET | SOT-23-6 | $0.10 | Power switching |

Total size: ~6x6mm per board. Requires SMD soldering skills.

---

## LiPo Battery Options

| Size Code | Dimensions | Capacity | Est. Runtime | Price |
|-----------|------------|----------|--------------|-------|
| 301020 | 10x20x3mm | 60mAh | ~15 min | $3 |
| 401120 | 11x20x4mm | 80mAh | ~20 min | $3 |
| **501220** | 12x20x5mm | 100mAh | ~25 min | $4 |
| 401230 | 12x30x4mm | 150mAh | ~40 min | $4 |
| **602025** | 20x25x6mm | 200mAh | ~50 min | $5 |

Recommended: **501220** (good balance of size/capacity) or **602025** (longer runtime)

---

## LiPo Battery Components

For portable operation with LiPo battery:

| Qty | Component | Purpose | Est. Price |
|-----|-----------|---------|------------|
| 1-2 | 1S LiPo Battery | TX: 500-1000mAh, RX: 100-200mAh | $4-8 |
| 1-2 | **1S LiPo Protection PCB** | **Over-discharge protection (1 per board)** | $0.30 ea |
| 4 | 10K Resistor | Voltage dividers (2 per board) | $0.20 |
| 2 | 100uF Electrolytic Cap | Power smoothing | $0.30 |
| 2 | 0.1uF Ceramic Cap | High-freq filtering | $0.10 |
| 2 | Slide Switch SPDT | Power on/off | $0.50 |
| 2 | JST-PH 2.0 Connector | Battery plug (optional) | $0.30 |

### Charging Modules (depends on board)

| Board | Built-in Charging? | Needs TP4056? |
|-------|-------------------|---------------|
| ESP32-C3 Super Mini | NO | Yes ($1) |
| Seeed XIAO ESP32-C3 | YES (battery pads) | No |

### Boost Converter (depends on setup)

| Component | When Needed | Est. Price |
|-----------|-------------|------------|
| MT3608 | RX on LiPo + 5V servo (SG90/MG90S) | $1 |

**NOT needed if:**
- RX powered by 5V adapter (recommended for standard build)
- OR using GS-1502 servo (3.7-5V compatible, recommended for compact build)

---

## Servo Options

### Standard Build
| Model | Size | Torque | Voltage | Price |
|-------|------|--------|---------|-------|
| SG90 | 23x12x29mm | 1.8kg-cm | 4.8-6V | $3 |
| MG90S | 23x12x29mm | 2.2kg-cm | 4.8-6V | $4 |

### Compact Build (Linear Servos)
| Model | Size | Stroke | Voltage | Price |
|-------|------|--------|---------|-------|
| **GS-1502** | 21x15x12mm | ~5mm | 3.7-5V | $3-4 |
| PZ-15320 | 23x12x6mm | 20mm | 4.8-6V | $8 |

**Recommended: GS-1502** - Works directly from 1S LiPo (no boost converter needed), ultra-light (1.5g)

---

## Motor Options

### Standard Build
| Type | Size | Voltage | RPM | Price |
|------|------|---------|-----|-------|
| TT Motor | 70x22x18mm | 3-6V | 200 | $2 |
| GA12-N20 | 10x12x25mm | 3-6V | 100-1000 | $3 |

### Compact Build
| Type | Size | Voltage | RPM | Price |
|------|------|---------|-----|-------|
| **N20 Micro** | 10x12x20mm | 3-6V | 100-600 | $2 |
| 6mm Coreless | 6x14mm | 3.7V | 50000 | $1 |
| 7mm Coreless | 7x16mm | 3.7V | 40000 | $1 |

Recommended: **N20 with gearbox** - good torque, reasonable size

---

## Complete Shopping List - Standard Build (TX + RX)

```
CORE COMPONENTS:
2x ESP32-C3 Super Mini
2x HW-504 Joystick Module
1x 1.9" OLED 128x64 SH1106 I2C (TX)
1x 1.3" OLED 128x64 SSD1306 I2C (RX)
1x MX1508 Dual Motor Driver
1x SG90 Servo
2x DC Motor 3-6V with gearbox
1x 2N2222 NPN Transistor
1x 1K Resistor 1/4W
2x USB-C Data Cable

FOR LIPO OPERATION (optional):
2x 1S LiPo Battery (TX: 500-1000mAh, RX: 1000-2000mAh)
2x 1S LiPo Protection PCB 5x10mm  <-- REQUIRED FOR SAFETY
4x 10K Resistor (voltage dividers)
2x TP4056 Charger Module (ESP32-C3 Super Mini has no built-in charging)
1x MT3608 Boost Converter (RX only, if LiPo + 5V servo)
2x Slide Switch SPDT
2x 100uF Electrolytic Capacitor
2x 0.1uF Ceramic Capacitor

RECOMMENDED:
1x 1000uF Electrolytic Capacitor (motor smoothing)
4x 0.1uF Ceramic Capacitor (ADC filtering)
1x 5V 2A+ USB-C Power Adapter
2x Breadboard 830pt
1x Jumper Wire Kit
```

---

## Complete Shopping List - Compact Build (RX only)

```
1x Seeed XIAO ESP32-C3
1x DRV8833 dual motor driver breakout
2x N20 micro gear motor 3-6V
1x GS-1502 linear servo (3.7-5V, no boost needed)
1x 501220 LiPo battery 100mAh (or 602025 200mAh)
1x 1S LiPo Protection PCB 5x10mm  <-- REQUIRED FOR SAFETY
2x 10K resistor 0402 or 0603
1x 100uF capacitor SMD
1x 30AWG silicone wire (1m each: red, black, colors)

OPTIONAL:
1x 0.42" OLED 72x40 I2C (debugging)
1x Slide switch (power on/off)

Note: GS-1502 works at 3.7V - no MT3608 boost converter needed!
```

---

## Where to Buy

| Component | Best Source |
|-----------|-------------|
| XIAO ESP32-C3 | Seeed Studio, Amazon |
| ESP32-C3 Super Mini | AliExpress |
| DRV8833 Breakout | AliExpress, Amazon |
| MX1508 | AliExpress |
| N20 Motors | AliExpress, Amazon |
| LiPo Batteries | AliExpress (search by size code) |
| **1S Protection PCB** | AliExpress ("1S 3.7V protection board") |
| OLED Displays | AliExpress, Amazon |
| **GS-1502 Servo** | AliExpress, Amazon ("GS-1502 linear servo") |
| SG90/MG90S Servos | Amazon, HobbyKing |
| Capacitors/Resistors | LCSC, Mouser, DigiKey |
| TP4056 Charger | AliExpress (not needed for XIAO - has built-in) |
| MT3608 Boost | AliExpress (not needed with GS-1502) |

---

## Tools Needed

| Tool | Purpose | Est. Price |
|------|---------|------------|
| Soldering Iron | Wiring connections | $20-50 |
| Multimeter | Verify voltages | $15-30 |
| Wire Strippers | 30AWG capable | $10 |
| Flush Cutters | Trimming leads | $8 |
| Tweezers | SMD components | $5 |
| Hot Glue Gun | Securing components | $10 |

---

**Document Version:** 1.3
**Last Updated:** 2026-01-29
**Changes:** Added GS-1502 as recommended servo for compact build (3.7-5V, no boost needed), updated costs
