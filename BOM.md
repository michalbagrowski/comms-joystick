# Bill of Materials (BOM)

## Quick Summary

| Build | Components | Est. Cost |
|-------|------------|-----------|
| Standard TX + RX | Full-size modules | ~$45 |
| Standard TX + Compact RX | Mixed | ~$35 |
| Compact RX only | Miniaturized | ~$25 |

---

## Standard Build (Full Size)

### Transmitter (TX) - ESP32-C3 Super Mini

| Qty | Component | Specs | Est. Price | Link/Notes |
|-----|-----------|-------|------------|------------|
| 1 | ESP32-C3 Super Mini | 22x18mm | $4 | AliExpress |
| 2 | HW-504 Joystick Module | Analog XY + button | $2 | |
| 1 | 1.9" OLED Display | 128x64, I2C, SH1106 | $4 | 3.3V only! |
| 1 | USB-C Cable | Data + power | $2 | |
| - | Jumper wires | Various | $2 | |

**TX Subtotal: ~$14**

### Receiver (RX) - Standard Build

| Qty | Component | Specs | Est. Price | Link/Notes |
|-----|-----------|-------|------------|------------|
| 1 | ESP32-C3 Super Mini | 22x18mm | $4 | AliExpress |
| 1 | MX1508 Motor Driver | Dual H-bridge | $2 | |
| 1 | SG90 Servo | Standard size | $3 | |
| 2 | DC Motors | 3-6V, with gearbox | $4 | |
| 1 | 1.3" OLED Display | 128x64, I2C, SSD1306 | $3 | 3.3V only! |
| 1 | 2N2222 NPN Transistor | Motor enable circuit | $0.20 | |
| 1 | 1K Resistor | 1/4W | $0.05 | |
| 1 | 5V 2A Power Adapter | USB-C or barrel | $5 | For motors |

**RX Standard Subtotal: ~$21**

### Optional - Standard Build

| Qty | Component | Purpose | Est. Price |
|-----|-----------|---------|------------|
| 1 | 1000µF Electrolytic Cap | Motor power smoothing | $0.50 |
| 2 | 100µF Electrolytic Cap | Power stability | $0.30 |
| 4 | 0.1µF Ceramic Cap | ADC filtering | $0.20 |
| 2 | Breadboard 830pt | Prototyping | $6 |

---

## Compact Build (Miniaturized RX)

### Transmitter (TX) - Same as Standard

Use standard TX build above (~$14)

### Receiver (RX) - Compact Build

| Qty | Component | Size | Est. Price | Link/Notes |
|-----|-----------|------|------------|------------|
| 1 | **Seeed XIAO ESP32-C3** | 21x17mm | $5 | [Seeed Studio](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html) |
| 1 | **DRV8833 Breakout** | 10x15mm | $2 | Search "DRV8833 module" |
| 2 | Micro DC Motors | N20 or smaller | $3 | Your choice |
| 1 | Micro Linear Servo | ~20x10mm | $8 | PZ-15320 or similar |
| 1 | LiPo Battery | 100-200mAh, 1S | $4 | 501220 or 602025 size |
| 2 | 10K Resistor | 0402 or 0603 SMD | $0.10 | Voltage divider |
| 1 | 100µF Capacitor | SMD or small | $0.20 | Power smoothing |
| - | 30AWG Silicone Wire | Thin, flexible | $2 | |

**RX Compact Subtotal: ~$25**

### Optional - Compact Build

| Qty | Component | Purpose | Est. Price |
|-----|-----------|---------|------------|
| 1 | 0.42" OLED Display | Debugging only | $4 |
| 1 | MT3608 Mini Boost | 5V for servo | $1 |
| 1 | Slide Switch | Power on/off | $0.50 |
| 1 | JST 1.25mm Connector | Battery plug | $0.30 |

---

## LiPo Battery Options (Compact Build)

| Size Code | Dimensions | Capacity | Est. Runtime | Price |
|-----------|------------|----------|--------------|-------|
| 301020 | 10x20x3mm | 60mAh | ~15 min | $3 |
| 401120 | 11x20x4mm | 80mAh | ~20 min | $3 |
| **501220** | 12x20x5mm | 100mAh | ~25 min | $4 |
| 401230 | 12x30x4mm | 150mAh | ~40 min | $4 |
| **602025** | 20x25x6mm | 200mAh | ~50 min | $5 |

Recommended: **501220** (good balance of size/capacity) or **602025** (longer runtime)

---

## LiPo Battery Components (Both Builds)

For portable operation with LiPo battery:

| Qty | Component | Purpose | Est. Price |
|-----|-----------|---------|------------|
| 1 | 1S LiPo Battery | TX: 500-1000mAh, RX: 100-200mAh | $4-8 |
| 4 | 10K Resistor | Voltage dividers (2 per board) | $0.20 |
| 2 | 100µF Electrolytic Cap | Power smoothing | $0.30 |
| 2 | 0.1µF Ceramic Cap | High-freq filtering | $0.10 |
| 1 | MT3608 Boost Converter | 5V for RX servo (standard build) | $1 |
| 1 | TP4056 Charger Module | LiPo charging (standard build) | $1 |

**Note:** XIAO ESP32-C3 has built-in LiPo charging - no TP4056 needed for compact build!

---

## Servo Options

### Standard Build
| Model | Size | Torque | Voltage | Price |
|-------|------|--------|---------|-------|
| SG90 | 23x12x29mm | 1.8kg·cm | 4.8-6V | $3 |
| MG90S | 23x12x29mm | 2.2kg·cm | 4.8-6V | $4 |

### Compact Build (Linear Servos)
| Model | Size | Stroke | Voltage | Price |
|-------|------|--------|---------|-------|
| PZ-15320 | 23x12x6mm | 20mm | 4.8-6V | $8 |
| Micro Linear | ~20x8mm | 10-15mm | 3-6V | $6-10 |

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

## Where to Buy

| Component | Best Source |
|-----------|-------------|
| XIAO ESP32-C3 | Seeed Studio, Amazon |
| ESP32-C3 Super Mini | AliExpress |
| DRV8833 Breakout | AliExpress, Amazon |
| MX1508 | AliExpress |
| N20 Motors | AliExpress, Amazon |
| LiPo Batteries | AliExpress (search by size code) |
| OLED Displays | AliExpress, Amazon |
| Servos | Amazon, HobbyKing |
| Capacitors/Resistors | LCSC, Mouser, DigiKey |

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

## Quick Order List (Compact Build)

Copy this for ordering:

```
1x Seeed XIAO ESP32-C3
1x DRV8833 dual motor driver breakout
2x N20 micro gear motor 3-6V (your RPM choice)
1x Micro linear servo (or PZ-15320)
1x 501220 LiPo battery 100mAh (or 602025 200mAh)
1x 0.42" OLED 72x40 I2C (optional)
2x 10K resistor 0402 or 0603
1x 100µF capacitor SMD
1x 30AWG silicone wire (1m each: red, black, other colors)
```

---

**Document Version:** 1.0
**Last Updated:** 2026-01-28
