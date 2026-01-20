# WiFi Setup Guide

**Last Updated:** 2026-01-19
**Project:** ESP32 Dual Joystick BLE/WiFi Controller

---

## Overview

This project supports two communication modes:
- **BLE Mode (Default):** Bluetooth Low Energy communication
- **WiFi Mode:** UDP over WiFi with mDNS discovery

This guide covers everything you need to know about using WiFi mode.

---

## Quick Start

### 1. Enable WiFi Mode

Edit both `transmitter/transmitter.ino` and `receiver/receiver.ino`:

```cpp
// Uncomment this line at the top of both files:
#define USE_WIFI
```

### 2. Configure WiFi Network

The default configuration (already set in code):
```cpp
#define WIFI_SSID "amplifi"
#define WIFI_PASSWORD "123qwe123"
```

To change the network, edit these values in both files.

### 3. Compile and Upload

Using Makefile (recommended):
```bash
make wifi-all
```

Or manually:
```bash
make wifi-compile
make wifi-upload-transmitter
make wifi-upload-receiver
```

### 4. Verify Connection

Open serial monitor to check connection:
```bash
make monitor-transmitter  # In one terminal
make monitor-receiver     # In another terminal
```

Look for:
- "WiFi Connected!"
- "Transmitter found at: X.X.X.X"
- UDP packets being sent/received

---

## WiFi vs BLE Comparison

| Feature | BLE Mode | WiFi Mode |
|---------|----------|-----------|
| **Range** | 10-30m | 50-100m |
| **Latency** | ~100-150ms | ~50-100ms |
| **Power Consumption** | Low | Higher |
| **Setup Complexity** | Simple (auto-pair) | Moderate (needs WiFi network) |
| **Interference** | Less susceptible | More susceptible (2.4GHz crowded) |
| **Multiple Receivers** | One-to-one | One-to-many (broadcast) |
| **Network Required** | No | Yes |

---

## How WiFi Mode Works

### Architecture

```
┌─────────────────┐              ┌─────────────────┐
│  Transmitter    │              │   Receiver      │
│  (TX)           │              │   (RX)          │
├─────────────────┤              ├─────────────────┤
│ 1. Read         │              │ 1. Connect to   │
│    Joysticks    │              │    WiFi         │
│                 │              │                 │
│ 2. Connect to   │              │ 2. Start UDP    │
│    WiFi         │              │    Listener     │
│                 │              │                 │
│ 3. Start mDNS   │              │ 3. Discover TX  │
│    Responder    │   WiFi       │    via mDNS     │
│                 ├─────────────>│                 │
│ 4. Broadcast    │   UDP Port   │ 4. Receive      │
│    UDP Packets  │   4210       │    Packets      │
│    @ 10Hz       │              │                 │
│                 │              │ 5. Control      │
│                 │              │    Servo        │
└─────────────────┘              └─────────────────┘
```

### Protocol Details

- **Transport:** UDP (User Datagram Protocol)
- **Port:** 4210
- **Packet Rate:** 10 Hz (100ms interval)
- **Packet Size:** 12 bytes
- **Discovery:** mDNS (esp32-joystick-tx.local)
- **Addressing:** Broadcast (255.255.255.255) or unicast after discovery

### Data Packet Structure

```cpp
struct JoystickData {
  int16_t joy1_x;    // 2 bytes (0-4095)
  int16_t joy1_y;    // 2 bytes (0-4095)
  uint8_t joy1_sw;   // 1 byte (0=pressed, 1=released)
  int16_t joy2_x;    // 2 bytes (0-4095)
  int16_t joy2_y;    // 2 bytes (0-4095)
  uint8_t joy2_sw;   // 1 byte (0=pressed, 1=released)
} // Total: 12 bytes
```

---

## Configuration

### Network Settings

**Location:** Top of both `.ino` files (inside `#ifdef USE_WIFI` block)

```cpp
#define WIFI_SSID "your-network-name"
#define WIFI_PASSWORD "your-password"
#define UDP_PORT 4210
#define MDNS_HOSTNAME "esp32-joystick-tx"
```

**Recommendations:**
- Use 2.4GHz WiFi (ESP32-C3 doesn't support 5GHz)
- Avoid special characters in SSID/password
- Use WPA2-PSK security (most compatible)
- Keep router within 10-20m for best performance

### Port Configuration

Default port: **4210**

To change (edit both files):
```cpp
#define UDP_PORT 9999  // Your custom port
```

**Note:** Ensure firewall allows UDP traffic on chosen port.

### mDNS Hostname

Default: **esp32-joystick-tx**

To change (transmitter only):
```cpp
#define MDNS_HOSTNAME "my-custom-name"
```

Receiver will discover at: `my-custom-name.local`

---

## Connection Flow

### Transmitter Startup

1. **Initialize Hardware**
   - Setup joystick pins
   - Initialize OLED display
   - Configure ADC

2. **Connect to WiFi**
   - Display: "TX: WiFi Init"
   - Connect to configured network
   - Wait up to 15 seconds
   - Display: "TX: WiFi OK" with IP address

3. **Start mDNS Responder**
   - Advertise as "esp32-joystick-tx.local"
   - Allows automatic discovery by receiver

4. **Start UDP Server**
   - Listen on port 4210
   - Begin sending joystick data every 100ms

5. **Normal Operation**
   - Display shows "WiFi" status indicator
   - Send UDP packets to broadcast address
   - LED blinks every 500ms

### Receiver Startup

1. **Initialize Hardware**
   - Setup servo on GPIO0
   - Initialize OLED display
   - Center servo at 90°

2. **Connect to WiFi**
   - Display: "RX: WiFi Init"
   - Connect to configured network
   - Wait up to 15 seconds
   - Display: "RX: WiFi OK" with IP address

3. **Discover Transmitter**
   - Display: "RX: Finding TX"
   - Query mDNS for "esp32-joystick-tx.local"
   - Retry up to 20 times (10 seconds)
   - If found: Display "RX: TX Found" with IP
   - If not found: Listen for broadcast packets

4. **Start UDP Listener**
   - Listen on port 4210
   - Wait for first packet
   - Store transmitter IP from first packet

5. **Normal Operation**
   - Display shows "WiFi" status indicator
   - Receive UDP packets (validated size)
   - Update display circles
   - Control servo based on Joy1 X-axis

---

## Display Indicators

### Transmitter Display (WiFi Mode)

```
┌──────────────────────────────────────────────────┐
│ J1    ○       J2    ○               WiFi        │  ← "WiFi" when connected
│       /│\            /│\                         │
│      ┼─○            ┼─○                          │
│                                                  │
│ 3515               3352                          │
└──────────────────────────────────────────────────┘
```

Status indicators:
- **"WiFi"** - Connected and sending
- **"----"** - WiFi disconnected

### Receiver Display (WiFi Mode)

```
┌──────────────────────────────────────────────────┐
│ J1  ○     J2  ○                     WiFi        │  ← "WiFi" when connected
│     /│\        /│\                               │
│    ┼─○        ┼─○                                │
│ S:90°                                            │
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░░░░░  │
└──────────────────────────────────────────────────┘
```

Status indicators:
- **"WiFi"** - Connected and receiving
- **"RX: Waiting..."** - Connected but no data
- **"RX: No WiFi"** - WiFi disconnected

---

## Troubleshooting

### WiFi Connection Issues

**Problem:** "WiFi connection failed"

**Solutions:**
1. **Verify network credentials**
   ```cpp
   #define WIFI_SSID "amplifi"      // Check exact name
   #define WIFI_PASSWORD "123qwe123" // Check exact password
   ```

2. **Check WiFi band**
   - ESP32-C3 only supports 2.4GHz
   - Ensure router has 2.4GHz enabled
   - Disable "5GHz only" mode

3. **Check signal strength**
   - Move closer to router
   - Remove obstacles (walls, metal objects)
   - Check router status lights

4. **Router settings**
   - Enable DHCP
   - Disable MAC filtering (or add ESP32 MAC)
   - Disable AP isolation (guest network)
   - Use WPA2-PSK security

5. **Serial monitor debugging**
   ```bash
   make monitor-transmitter
   ```
   Look for:
   - "Connecting to WiFi..."
   - IP address assignment
   - Error messages

### mDNS Discovery Failures

**Problem:** "Transmitter discovery failed"

**Solutions:**
1. **mDNS/Bonjour support**
   - Some routers block mDNS traffic
   - Try disabling "mDNS filtering"
   - Enable "Bonjour forwarding"

2. **Network isolation**
   - Ensure both boards on same network/VLAN
   - Check "Client Isolation" is disabled
   - Guest networks often block peer-to-peer

3. **Fallback to broadcast**
   - Receiver will automatically fall back
   - Listens for any broadcast packets on port 4210
   - Will detect transmitter from first packet received

4. **Manual IP configuration** (future enhancement)
   - Currently not implemented
   - Broadcast mode works without mDNS

### No Data Received

**Problem:** Receiver shows "Waiting..." indefinitely

**Solutions:**
1. **Verify transmitter is sending**
   ```bash
   make monitor-transmitter
   ```
   Should see joystick values printing

2. **Check same network**
   - Both must be on same WiFi network
   - Check IP addresses in same subnet
   - Example: 192.168.1.X (good), 192.168.1.X and 192.168.2.X (bad)

3. **Firewall/security**
   - Router firewall may block UDP
   - Try disabling "WiFi Isolation"
   - Check router's security settings

4. **Port conflict**
   - Another device may be using port 4210
   - Change UDP_PORT to different value (e.g., 5555)
   - Update both transmitter and receiver

### Packet Loss / Jitter

**Problem:** Display updates are choppy or intermittent

**Solutions:**
1. **Signal strength**
   - Move closer to router
   - Check WiFi signal strength indicator
   - Reduce obstacles between boards and router

2. **Network congestion**
   - Too many devices on WiFi
   - Disable bandwidth-heavy devices
   - Use 5GHz for other devices (leave 2.4GHz for ESP32)

3. **Interference**
   - Microwave ovens (2.4GHz interference)
   - Bluetooth devices
   - Other WiFi networks on same channel
   - Try changing router WiFi channel

4. **Software filtering**
   - Existing ADC filtering helps smooth data
   - Adjust SMOOTHING_ALPHA in transmitter (0.1-0.5)
   - Lower values = smoother but slower response

### ESP32 Resets / Crashes

**Problem:** Board reboots randomly in WiFi mode

**Solutions:**
1. **Power supply**
   - WiFi uses more power than BLE
   - Use good quality USB cable
   - Try external 5V power supply (1A+)

2. **Servo power**
   - Servo can draw significant current
   - Use separate 5V supply for servo
   - Add 100µF capacitor across servo power

3. **Memory issues**
   - WiFi mode uses more RAM (12% vs 7%)
   - Check Serial monitor for "heap" errors
   - Reduce buffer sizes if needed

---

## Performance Optimization

### Reducing Latency

1. **Increase update rate**
   ```cpp
   // In both files, reduce delay:
   if (millis() - lastUpdate > 50) {  // Was 100ms, now 50ms (20Hz)
   ```

2. **Disable filtering** (if latency critical)
   ```cpp
   #define ADC_SAMPLES 1         // Was 4
   #define SMOOTHING_ALPHA 1.0   // Was 0.3 (disables smoothing)
   ```

3. **Router QoS**
   - Enable QoS (Quality of Service)
   - Prioritize UDP traffic
   - Set ESP32 devices as "high priority"

### Improving Reliability

1. **Add packet sequencing** (future enhancement)
   ```cpp
   struct JoystickData {
     uint32_t sequence;  // Add packet counter
     // ... existing fields
   };
   ```

2. **Add timeout detection**
   - Detect if no packets received for 1 second
   - Display warning on receiver
   - Auto-reconnect logic

3. **Retry logic**
   - Retransmit important packets (button presses)
   - TCP instead of UDP (but higher latency)

### Power Optimization

1. **Reduce WiFi power**
   ```cpp
   // Add to setup() in both files:
   WiFi.setSleep(true);  // Enable WiFi sleep when idle
   esp_wifi_set_ps(WIFI_PS_MIN_MODEM);  // Minimum power save
   ```

2. **Lower update rate**
   ```cpp
   if (millis() - lastUpdate > 200) {  // 5Hz instead of 10Hz
   ```

3. **Disable mDNS after discovery**
   ```cpp
   // In receiver, after transmitter found:
   MDNS.end();  // Stop mDNS to save power
   ```

---

## Advanced Configuration

### Multiple Receivers

WiFi mode supports one-to-many communication via broadcast.

**Setup:**
1. Flash multiple receivers with same code
2. All receivers will listen on port 4210
3. All receive same joystick data simultaneously

**Use Cases:**
- Multiple robots controlled by one transmitter
- Display boards showing joystick position
- Data logging stations

**Limitations:**
- All receivers get same data (no filtering)
- No feedback from receivers to transmitter
- Increase network traffic linearly

### Static IP Configuration

Avoid DHCP delays by using static IPs.

**Add to WiFi setup (after WiFi.begin()):**
```cpp
IPAddress local_IP(192, 168, 1, 100);    // ESP32 IP
IPAddress gateway(192, 168, 1, 1);       // Router IP
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

if (!WiFi.config(local_IP, gateway, subnet, dns)) {
  Serial.println("Static IP configuration failed");
}
```

**Transmitter:** 192.168.1.100
**Receiver:** 192.168.1.101

### Unicast Instead of Broadcast

More efficient if you know receiver IP.

**In transmitter, after WiFi connected:**
```cpp
IPAddress receiverIP(192, 168, 1, 101);  // Replace broadcast
udp.beginPacket(receiverIP, UDP_PORT);   // Instead of broadcastIP
```

**Advantages:**
- Less network traffic
- More secure (not broadcasting)
- Slightly lower latency

**Disadvantages:**
- Must know receiver IP
- No automatic discovery
- One transmitter → one receiver

### Custom Data Packets

Add more data to the packet.

**Example: Add battery voltage**
```cpp
struct JoystickData {
  int16_t joy1_x;
  int16_t joy1_y;
  uint8_t joy1_sw;
  int16_t joy2_x;
  int16_t joy2_y;
  uint8_t joy2_sw;
  uint16_t battery_mv;  // Add battery voltage in millivolts
  uint8_t checksum;     // Add checksum for validation
} // Total: 15 bytes
```

**Note:** Update packet size validation on receiver:
```cpp
if (packetSize == sizeof(joystickData)) {  // Now 15 instead of 12
```

---

## Security Considerations

### Current Security Level

- **Network:** WPA2-PSK encrypted WiFi
- **Data:** Unencrypted UDP packets
- **Access:** Anyone on same network can receive
- **Spoofing:** No authentication of transmitter

### Recommendations

1. **Use WPA2-PSK**
   - Minimum security level
   - Protects against casual sniffing
   - Use strong password (20+ characters)

2. **Isolated Network**
   - Dedicated WiFi network for ESP32 devices
   - No internet access required
   - Guest network with client isolation disabled

3. **MAC Filtering**
   - Router setting to allow only known devices
   - Add both ESP32 MAC addresses to whitelist
   - Check Serial monitor for MAC address

4. **Future Enhancements**
   - Encrypt UDP payload (AES)
   - Add packet authentication (HMAC)
   - Use TLS/DTLS for secure channel
   - Implement challenge-response authentication

---

## Debugging Tips

### Enable Verbose Logging

Add to `setup()`:
```cpp
Serial.setDebugOutput(true);  // Enable ESP32 debug output
```

### Monitor WiFi Events

Add event handler:
```cpp
void WiFiEvent(WiFiEvent_t event) {
  Serial.print("WiFi Event: ");
  Serial.println(event);
}

// In setup():
WiFi.onEvent(WiFiEvent);
```

### Packet Inspection

Log every packet sent/received:
```cpp
// Transmitter:
Serial.printf("TX: %d,%d,%d,%d\n", joystickData.joy1_x,
  joystickData.joy1_y, joystickData.joy2_x, joystickData.joy2_y);

// Receiver:
Serial.printf("RX: Packet from %s, size %d\n",
  udp.remoteIP().toString().c_str(), packetSize);
```

### Network Scanning

Use WiFi scanner to check available networks:
```cpp
int n = WiFi.scanNetworks();
for (int i = 0; i < n; i++) {
  Serial.printf("%d: %s (%d) %s\n", i+1,
    WiFi.SSID(i).c_str(), WiFi.RSSI(i),
    WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Encrypted");
}
```

---

## Switching Between Modes

### BLE to WiFi

1. **Edit both `.ino` files:**
   ```cpp
   // Uncomment this line:
   #define USE_WIFI
   ```

2. **Compile and upload:**
   ```bash
   make wifi-all
   ```

3. **Verify display shows "WiFi" instead of "TX"/"RX"**

### WiFi to BLE

1. **Edit both `.ino` files:**
   ```cpp
   // Comment this line:
   // #define USE_WIFI
   ```

2. **Compile and upload:**
   ```bash
   make all
   ```

3. **Verify display shows "TX"/"RX" instead of "WiFi"**

### No Changes Required

- Pin assignments stay the same
- Servo control stays the same
- Display layout stays the same
- Joystick calibration stays the same

---

## FAQ

**Q: Can I use both BLE and WiFi simultaneously?**
A: No. They are compile-time mutually exclusive. Choose one mode.

**Q: Which mode is better?**
A: BLE for simplicity and low power. WiFi for range and latency.

**Q: Can I switch modes without recompiling?**
A: Not currently. Future enhancement could add runtime switching.

**Q: Does WiFi mode work without internet?**
A: Yes. Only local network required. Router doesn't need internet.

**Q: What's the maximum range?**
A: 50-100m with good WiFi router. Depends on obstacles and interference.

**Q: How many receivers can I have?**
A: Unlimited (broadcast mode). All receive same data.

**Q: Can receiver send data back to transmitter?**
A: Not currently implemented. UDP is one-way in this design.

**Q: Is the data encrypted?**
A: No. WiFi network is encrypted (WPA2), but UDP payload is plain text.

**Q: Can I use 5GHz WiFi?**
A: No. ESP32-C3 only supports 2.4GHz.

**Q: What if mDNS doesn't work?**
A: Receiver automatically falls back to listening for broadcast packets.

---

## References

- [ESP32 WiFi Library](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
- [ESP32 UDP Documentation](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/examples/WiFiUDPClient)
- [mDNS Protocol](https://en.wikipedia.org/wiki/Multicast_DNS)
- [UDP Protocol](https://en.wikipedia.org/wiki/User_Datagram_Protocol)
- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)

---

## Changelog

**2026-01-19:**
- Initial WiFi implementation
- UDP communication on port 4210
- mDNS discovery with broadcast fallback
- Compile-time mode switching
- Complete documentation

---

*For BLE mode documentation, see README.md and CLAUDE.md*
