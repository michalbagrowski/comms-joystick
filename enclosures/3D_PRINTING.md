# 3D Printed Enclosures - ESP32 Joystick Project

## Overview

This folder contains parametric OpenSCAD designs for:
1. **TX Controller** - Handheld transmitter with dual joysticks
2. **RX Car Chassis** - Hot Wheels-sized RC car body

Both designs are parametric - adjust the component dimensions in the `.scad` files to match your actual parts.

---

## TX Controller Enclosure

### Description
Ergonomic handheld controller housing:
- 2x HW-504 joystick modules
- ESP32-C3 Super Mini
- 1.9" OLED display (SH1106)
- 500-1000mAh LiPo battery
- USB-C port access for charging/programming

### Dimensions
- **Exterior:** ~160 x 95 x 45mm
- **Weight:** ~80g (printed)

### Files
| File | Description |
|------|-------------|
| `tx/tx_controller.scad` | OpenSCAD source (parametric) |
| `tx/tx_bottom.stl` | Bottom case half |
| `tx/tx_top.stl` | Top lid with joystick holes |

### Print Settings - TX Controller

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Layer Height** | 0.2mm | 0.16mm for finer detail |
| **Infill** | 20% | Grid or gyroid pattern |
| **Perimeters** | 3 | For strength |
| **Top/Bottom Layers** | 4 | Solid surfaces |
| **Supports** | YES | Required for joystick holes, USB port |
| **Support Type** | Tree supports | Easier removal |
| **Build Plate Adhesion** | Brim | 5mm for bed adhesion |
| **Material** | PLA or PETG | PETG for durability |
| **Nozzle** | 0.4mm | Standard |
| **Print Time** | ~4-5 hours | Both parts |

### Print Orientation

**Bottom Case:**
- Print with interior facing UP
- Supports needed inside for screw posts

**Top Lid:**
- Print with exterior facing UP
- Supports needed for joystick hole overhangs

### Post-Processing

1. Remove supports carefully
2. Test-fit joystick modules before assembly
3. Sand joystick holes if too tight
4. Clean USB port opening

---

## RX Car Chassis

### Description
Compact RC car chassis (Hot Wheels scale):
- Seeed XIAO ESP32-C3
- DRV8833 motor driver
- GS-1502 linear servo (steering)
- 2x N20 gear motors
- 100-200mAh LiPo battery
- 20mm wheels (printable or purchased)

### Dimensions
- **Exterior:** 80 x 32 x 18mm (body only)
- **With wheels:** 80 x 48 x 28mm
- **Weight:** ~25g (printed, without electronics)

### Files
| File | Description |
|------|-------------|
| `rx_car/rx_car_chassis.scad` | OpenSCAD source (parametric) |
| `rx_car/chassis_bottom.stl` | Main chassis body |
| `rx_car/chassis_top.stl` | Top lid |
| `rx_car/wheel.stl` | Printable wheel (x4) |

### Print Settings - Car Chassis

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Layer Height** | 0.16mm | Fine detail for small parts |
| **Infill** | 15% | Lightweight |
| **Perimeters** | 2-3 | Balance of strength/weight |
| **Top/Bottom Layers** | 3 | Solid surfaces |
| **Supports** | YES | For wheel wells, motor mounts |
| **Support Type** | Normal | Or tree for easier removal |
| **Build Plate Adhesion** | Brim | 3mm |
| **Material** | PLA | Light weight priority |
| **Nozzle** | 0.4mm | 0.3mm for finer detail |
| **Print Time** | ~2-3 hours | All parts |

### Print Settings - Wheels

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Layer Height** | 0.12mm | Smooth surface |
| **Infill** | 25% | Strength for grip |
| **Perimeters** | 4 | Durable rim |
| **Supports** | NO | Print flat |
| **Material** | TPU | For grip (or PLA) |

### Print Orientation

**Chassis Bottom:**
- Print UPSIDE DOWN (wheel wells facing up)
- Better overhang quality for wheel arches

**Chassis Top:**
- Print flat (exterior up)
- No supports needed

**Wheels:**
- Print flat on build plate
- 4 wheels per print

### Post-Processing

1. Remove supports from wheel wells
2. Clean motor mount cavities
3. Test-fit components before final assembly
4. Add rubber bands to wheels for grip (if PLA)

---

## Material Recommendations

| Material | Pros | Cons | Best For |
|----------|------|------|----------|
| **PLA** | Easy to print, cheap | Brittle, heat sensitive | Prototypes, wheels |
| **PETG** | Durable, heat resistant | Stringing | TX case, final builds |
| **ABS** | Strong, heat resistant | Warping, fumes | Outdoor use |
| **TPU** | Flexible, grippy | Hard to print | Wheel tires |

### Color Suggestions
- **TX Controller:** Black or dark gray (hides wear)
- **Car Body:** Any color! Consider clear for showing off electronics
- **Wheels:** Black TPU for realism, or bright colors

---

## Assembly Instructions

### TX Controller Assembly

1. **Prepare components:**
   - Solder wires to ESP32, display, battery
   - Test electronics before enclosing

2. **Install battery:**
   - Place in battery compartment (bottom case)
   - Route wires to ESP32 location

3. **Install ESP32:**
   - Snap into mount
   - USB-C port aligned with side opening

4. **Install display:**
   - Fit into lid recess (from inside)
   - Secure with small amount of hot glue

5. **Install joysticks:**
   - Feed wires through holes
   - Screw into mount posts (M3x8mm screws)

6. **Wire connections:**
   - Connect all wires per WIRING.md
   - Bundle excess wire in center cavity

7. **Close case:**
   - Align top and bottom
   - Insert M3x12mm screws in corners

### Car Chassis Assembly

1. **Install motors:**
   - Press N20 motors into mounts
   - Ensure shafts exit through holes
   - Route wires to center

2. **Install servo:**
   - Place GS-1502 in front mount
   - Route wire to center

3. **Install battery:**
   - Place in compartment
   - Route wires to XIAO

4. **Install electronics:**
   - Place DRV8833 in mount
   - Place XIAO in mount (USB accessible from side)

5. **Wire connections:**
   - Connect per WIRING.md
   - Keep wires organized

6. **Install axles and wheels:**
   - Insert 2mm steel rod through axle holes
   - Press wheels onto axle ends

7. **Close chassis:**
   - Align lid
   - Insert M2x8mm screws

---

## Hardware Required

### TX Controller
| Item | Qty | Notes |
|------|-----|-------|
| M3x8mm screws | 4 | Joystick mounting |
| M3x12mm screws | 4 | Case closure |
| Slide switch 6mm | 1 | Power (optional) |

### Car Chassis
| Item | Qty | Notes |
|------|-----|-------|
| M2x8mm screws | 2 | Lid closure |
| 2mm steel rod | 2 | 50mm length, axles |
| Rubber O-rings | 4 | 18mm OD, wheel grip (optional) |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Parts don't fit | Measure components, adjust SCAD parameters |
| Joystick holes too tight | Sand with round file, or scale hole +2% |
| Wheels wobble | Check axle hole diameter, add shims |
| USB port blocked | Widen opening with hobby knife |
| Supports hard to remove | Use tree supports, lower support density |
| Layer separation | Increase print temperature, slow down |
| Warping | Use brim, heated bed (60°C PLA, 80°C PETG) |

---

## Customization

### Changing Component Sizes

Edit the parameters at the top of each `.scad` file:

```openscad
// Example: Changing battery size
batt_width = 35;   // Change these values
batt_depth = 25;
batt_height = 5;
```

Then re-render and export STL.

### Adding Features

Common modifications:
- Add LED windows
- Add antenna mount
- Add lanyard hole to TX
- Add body shell mounts to car

### Scaling

**Do NOT scale STL files directly** - component mounts won't fit.
Instead, adjust parameters in OpenSCAD for each component.

---

## Rendering STL Files

### Using OpenSCAD

1. Open `.scad` file in OpenSCAD
2. Uncomment the part you want to render:
   ```openscad
   // Bottom only (for printing)
   case_bottom();
   ```
3. Press F6 to render
4. File → Export → STL

### Using Command Line

```bash
# TX Controller bottom
openscad -o tx_bottom.stl -D 'part="bottom"' tx_controller.scad

# TX Controller top
openscad -o tx_top.stl -D 'part="top"' tx_controller.scad

# Car chassis
openscad -o chassis_bottom.stl -D 'part="bottom"' rx_car_chassis.scad
openscad -o chassis_top.stl -D 'part="top"' rx_car_chassis.scad
openscad -o wheel.stl -D 'part="wheel"' rx_car_chassis.scad
```

---

## Visualization Images

See `images/` folder for:
- `tx_controller_render.png` - TX controller render
- `tx_controller_exploded.png` - Exploded view
- `rx_car_render.png` - Car chassis render
- `rx_car_exploded.png` - Exploded view
- `assembly_diagram.png` - Assembly overview

Generate your own renders in OpenSCAD:
- View → Animate to rotate view
- File → Export → Image

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-29 | Initial release |

---

## License

These designs are provided under MIT License.
Free to use, modify, and distribute.

**Attribution appreciated but not required.**
