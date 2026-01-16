# Quick Start Guide

## Wiring Instructions

### Transmitter Board Connections

Connect components to the ESP32-C3 Super Mini as follows:

```
OLED Display (Pin order: GND, VCC, SCK, SDA):
┌─────────────┬─────────────┐
│ OLED Pin    │ ESP32-C3    │
├─────────────┼─────────────┤
│ GND         │ GND         │
│ VCC         │ 3.3V        │
│ SCK         │ GPIO7       │
│ SDA         │ GPIO6       │
└─────────────┴─────────────┘

Joystick 1 (Pin order: GND, 5V, VRX, VRY, SW):
┌─────────────┬─────────────┐
│ Joy Pin     │ ESP32-C3    │
├─────────────┼─────────────┤
│ GND         │ GND         │
│ 5V          │ 5V or 3.3V  │
│ VRX         │ GPIO0       │
│ VRY         │ GPIO1       │
│ SW          │ GPIO2       │
└─────────────┴─────────────┘

Joystick 2 (Pin order: GND, 5V, VRX, VRY, SW):
┌─────────────┬─────────────┐
│ Joy Pin     │ ESP32-C3    │
├─────────────┼─────────────┤
│ GND         │ GND         │
│ 5V          │ 5V or 3.3V  │
│ VRX         │ GPIO3       │
│ VRY         │ GPIO4       │
│ SW          │ GPIO5       │
└─────────────┴─────────────┘

Built-in LED: GPIO8 (no wiring needed)
```

### Receiver Board Connections

```
OLED Display (Pin order: GND, VCC, SCK, SDA):
┌─────────────┬─────────────┐
│ OLED Pin    │ ESP32-C3    │
├─────────────┼─────────────┤
│ GND         │ GND         │
│ VCC         │ 3.3V        │
│ SCK         │ GPIO7       │
│ SDA         │ GPIO6       │
└─────────────┴─────────────┘
```

## Upload Instructions

### Step 1: Find Your Serial Ports

Connect both ESP32 boards and run:
```bash
make list-ports
```

You should see something like:
```
Port               Type              Board Name  FQBN  Core
/dev/cu.wchusbserial-1430  Serial Port (USB)  Unknown
/dev/cu.wchusbserial-1440  Serial Port (USB)  Unknown
```

Note down both port names.

### Step 2: Upload to Transmitter

Replace `/dev/cu.wchusbserial-1430` with your actual port:

```bash
make upload-transmitter TX_PORT=/dev/cu.wchusbserial-1430
```

If upload fails, hold the BOOT button on the ESP32-C3 while uploading.

### Step 3: Upload to Receiver

Replace `/dev/cu.wchusbserial-1440` with your actual port:

```bash
make upload-receiver RX_PORT=/dev/cu.wchusbserial-1440
```

If upload fails, hold the BOOT button on the ESP32-C3 while uploading.

## Testing

1. Power on both boards
2. The transmitter LED should start blinking
3. Transmitter display shows: "TX: Waiting..."
4. Receiver display shows: "RX: Scanning..."
5. After a few seconds, both should show "Connected"
6. Move the joysticks and watch the displays update

## Troubleshooting

### Upload Fails
- Hold BOOT button during upload
- Check USB cable (must support data)
- Verify correct port selected

### OLED Display Blank
- Check wiring: SCK→GPIO7, SDA→GPIO6 (in pin order: GND, VCC, SCK, SDA)
- Verify 3.3V power connection on VCC pin (pin 2)
- Do NOT swap pins - code is configured correctly for your display

### No BLE Connection
- Reset both boards
- Keep boards within 10 meters
- Check serial monitor for debug info:
  ```bash
  make monitor-transmitter TX_PORT=/dev/cu.xxx
  ```

### Joysticks Not Responding
- Verify analog pins (GPIO0,1,3,4)
- Check joystick power (3.3V or 5V)
- Open serial monitor to see raw values

## Serial Monitor Commands

To debug the transmitter:
```bash
make monitor-transmitter TX_PORT=/dev/cu.wchusbserial-1430
```

To debug the receiver:
```bash
make monitor-receiver RX_PORT=/dev/cu.wchusbserial-1440
```

Press Ctrl+C to exit.

## Expected Display Output

### Transmitter:
```
TX: Connected
J1: CENTER
J2: RIGHT
J1: PRESSED
```

### Receiver:
```
RX: Connected
J1: CENTER
J2: RIGHT
J1: PRESSED
```

Both displays should show the same joystick data.
