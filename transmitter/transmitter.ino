// ========================================
// COMPILE-TIME COMMUNICATION MODE SWITCH
// ========================================
// Uncomment the line below to use WiFi instead of BLE
// #define USE_WIFI  // Comment out to use BLE mode

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#ifdef USE_WIFI
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include <ESPmDNS.h>
#else
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>
#endif

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 6
#define SCL_PIN 7

#define JOY1_VRX 0
#define JOY1_VRY 1
#define JOY1_SW 2

#define JOY2_VRX 3
#define JOY2_VRY 4
#define JOY2_SW 5

#define LED_PIN 8

// ========================================
// BATTERY MONITORING (LiPo)
// ========================================
// Voltage divider: LiPo+ --[10K]--+--[10K]-- GND
//                                 |
//                              GPIO10
#define VBAT_PIN 10
#define VBAT_DIVIDER 2.0          // Voltage divider ratio (10K/10K)
#define VBAT_FULL 4.2             // Fully charged voltage
#define VBAT_EMPTY 3.3            // Empty voltage (0% display threshold)
#define VBAT_WARNING 3.5          // Low battery warning threshold
#define VBAT_CUTOFF 3.2           // Shutdown threshold (protect LiPo)
#define VBAT_SAMPLES 10           // ADC samples for averaging

// Joystick 1 center values (measured at rest)
#define JOY1_CENTER_X 2235
#define JOY1_CENTER_Y 2217

// Joystick 2 center values (measured at rest)
#define JOY2_CENTER_X 2215
#define JOY2_CENTER_Y 2255

// ========================================
// COMMUNICATION CONFIGURATION
// ========================================
#ifdef USE_WIFI
  // WiFi Configuration
  #define WIFI_SSID "amplifi"
  #define WIFI_PASSWORD "123qwe123"
  #define UDP_PORT 4210
  #define MDNS_HOSTNAME "esp32-joystick-tx"
#else
  // BLE Configuration
  #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
  #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#endif

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========================================
// COMMUNICATION GLOBALS
// ========================================
#ifdef USE_WIFI
  WiFiUDP udp;
  IPAddress broadcastIP(255, 255, 255, 255); // Broadcast address
  volatile bool wifiConnected = false;
#else
  BLEServer* pServer = NULL;
  BLECharacteristic* pCharacteristic = NULL;
  bool deviceConnected = false;
  bool oldDeviceConnected = false;
#endif

struct JoystickData {
  int16_t joy1_x;
  int16_t joy1_y;
  uint8_t joy1_sw;
  int16_t joy2_x;
  int16_t joy2_y;
  uint8_t joy2_sw;
} joystickData;

// Battery monitoring
float batteryVoltage = 4.2;  // Current battery voltage
bool lowBatteryWarning = false;

// Read battery voltage with averaging
float readBatteryVoltage() {
  long sum = 0;
  for (int i = 0; i < VBAT_SAMPLES; i++) {
    sum += analogRead(VBAT_PIN);
    delayMicroseconds(100);
  }
  float avgRaw = (float)sum / VBAT_SAMPLES;
  return (avgRaw / 4095.0) * 3.3 * VBAT_DIVIDER;
}

// Get battery percentage
int getBatteryPercent() {
  if (batteryVoltage >= VBAT_FULL) return 100;
  if (batteryVoltage <= VBAT_EMPTY) return 0;
  return (int)((batteryVoltage - VBAT_EMPTY) / (VBAT_FULL - VBAT_EMPTY) * 100);
}

// Draw battery icon on display
void drawBatteryIcon(int x, int y) {
  int percent = getBatteryPercent();

  // Battery outline (16x8 pixels)
  display.drawRect(x, y, 14, 8, SH110X_WHITE);
  display.fillRect(x + 14, y + 2, 2, 4, SH110X_WHITE);

  // Fill level (0-3 bars)
  int bars = (percent + 16) / 33;
  if (bars > 0) display.fillRect(x + 2, y + 2, 3, 4, SH110X_WHITE);
  if (bars > 1) display.fillRect(x + 6, y + 2, 3, 4, SH110X_WHITE);
  if (bars > 2) display.fillRect(x + 10, y + 2, 2, 4, SH110X_WHITE);

  // Blink if low battery
  if (lowBatteryWarning && (millis() / 500) % 2 == 0) {
    display.fillRect(x, y, 14, 8, SH110X_WHITE);
  }
}

// Smoothing filter - exponential moving average
// Alpha = 0.3 means 30% new value, 70% old value (adjust 0.1-0.5)
#define SMOOTHING_ALPHA 0.3

// Oversampling - read ADC multiple times and average
#define ADC_SAMPLES 4

// Filtered values
float filtered_j1x = 0;
float filtered_j1y = 0;
float filtered_j2x = 0;
float filtered_j2y = 0;
bool firstRead = true;

// Function to read ADC with oversampling
int readADC(int pin) {
  long sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100); // Small delay between samples
  }
  return sum / ADC_SAMPLES;
}

// ========================================
// COMMUNICATION CALLBACKS
// ========================================
#ifndef USE_WIFI
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};
#endif

String getDirection(int x, int y, int centerX, int centerY) {
  int threshold = 1500;

  int dx = x - centerX;
  int dy = y - centerY;

  if (abs(dx) < threshold && abs(dy) < threshold) return "CENTER";

  if (abs(dx) > abs(dy)) {
    return (dx > 0) ? "RIGHT" : "LEFT";
  } else {
    return (dy > 0) ? "DOWN" : "UP";
  }
}

void drawJoystick(int16_t joyX, int16_t joyY, uint8_t button, int centerX, int centerY, int radius, const char* label, int joyCenterX, int joyCenterY) {
  display.drawCircle(centerX, centerY, radius, SH110X_WHITE);

  // Zero state position (bottom-right corner of circle)
  int zeroX = centerX + (radius - 3);
  int zeroY = centerY + (radius - 3);

  // Draw crosshairs at zero state position
  display.drawFastHLine(zeroX - 4, zeroY, 8, SH110X_WHITE);
  display.drawFastVLine(zeroX, zeroY - 4, 8, SH110X_WHITE);

  // Calculate offset from joystick center
  int offsetX = joyX - joyCenterX;
  int offsetY = joyY - joyCenterY;

  // Map to screen coordinates: offset=0 should appear at bottom-right corner
  // Full left (offset=-joyCenterX) should map to -2*(radius-3) from zero
  // Full right (offset=4095-joyCenterX) should map to 0 from zero
  int mapX = map(offsetX, -joyCenterX, 4095 - joyCenterX, -2*(radius - 3), 0);
  int mapY = map(offsetY, -joyCenterY, 4095 - joyCenterY, -2*(radius - 3), 0);

  // Position dot relative to zero state (bottom-right corner)
  int dotX = zeroX + mapX;
  int dotY = zeroY + mapY;

  // Constrain to circle boundary
  int dx = dotX - centerX;
  int dy = dotY - centerY;
  int distSq = dx * dx + dy * dy;
  int maxDistSq = (radius - 3) * (radius - 3);

  if (distSq > maxDistSq) {
    float angle = atan2(dy, dx);
    dotX = centerX + (radius - 3) * cos(angle);
    dotY = centerY + (radius - 3) * sin(angle);
  }

  if (button == LOW) {
    display.fillCircle(dotX, dotY, 2, SH110X_WHITE);
  } else {
    display.drawCircle(dotX, dotY, 2, SH110X_WHITE);
  }

  display.setTextSize(1);
  int labelX = centerX - 6;
  int labelY = 0;
  display.setCursor(labelX, labelY);
  display.print(label);
}

// Calculate and draw "fake" servo angle based on Joy1 X-axis
void drawFakeServoAngle(int16_t joy1_x) {
  // Calculate fake servo angle (same algorithm as receiver)
  int servo_angle = map(joy1_x, 0, 4095, 0, 180);
  servo_angle = constrain(servo_angle, 0, 180);

  // Servo angle text
  display.setCursor(0, 32);
  display.setTextSize(1);
  display.print("S:");
  display.print(servo_angle);
  display.print((char)247);  // Degree symbol

  // Servo position bar (horizontal, fills left-to-right)
  int barY = 40;
  display.drawRect(24, barY, 100, 6, SH110X_WHITE);
  int barWidth = map(servo_angle, 0, 180, 2, 98);
  display.fillRect(25, barY + 1, barWidth, 4, SH110X_WHITE);
}

// Calculate and draw "fake" motor speeds based on Joy2 values
void drawFakeMotorSpeeds(int16_t joy2_x, int16_t joy2_y) {
  // Calculate fake motor speeds (same algorithm as receiver)
  #define MOTOR_DEADZONE 200

  int motor1_speed = 0;
  int motor2_speed = 0;

  // Motor 1 from Joy2 X-axis
  int joy2x_offset = joy2_x - JOY2_CENTER_X;
  if (abs(joy2x_offset) < MOTOR_DEADZONE) {
    motor1_speed = 0;
  } else {
    motor1_speed = map(joy2_x, 0, 4095, -255, 255);
    motor1_speed = constrain(motor1_speed, -255, 255);
  }

  // Motor 2 from Joy2 Y-axis
  int joy2y_offset = joy2_y - JOY2_CENTER_Y;
  if (abs(joy2y_offset) < MOTOR_DEADZONE) {
    motor2_speed = 0;
  } else {
    motor2_speed = map(joy2_y, 0, 4095, -255, 255);
    motor2_speed = constrain(motor2_speed, -255, 255);
  }

  // Draw Motor 1 bar (horizontal, centered at y=50)
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.print("M1:");

  int bar1_center = 64;
  int bar1_width = map(abs(motor1_speed), 0, 255, 0, 45);

  // Draw center line
  display.drawFastVLine(bar1_center, 50, 6, SH110X_WHITE);

  // Draw bar
  if (motor1_speed > 0) {
    // Right side (forward)
    display.fillRect(bar1_center + 1, 51, bar1_width, 4, SH110X_WHITE);
    if (motor1_speed > 0) display.drawChar(bar1_center + bar1_width + 3, 48, '>', SH110X_WHITE, SH110X_BLACK, 1);
  } else if (motor1_speed < 0) {
    // Left side (reverse)
    display.fillRect(bar1_center - bar1_width, 51, bar1_width, 4, SH110X_WHITE);
    if (motor1_speed < 0) display.drawChar(bar1_center - bar1_width - 7, 48, '<', SH110X_WHITE, SH110X_BLACK, 1);
  }

  // Draw Motor 2 bar (horizontal, centered at y=58)
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print("M2:");

  int bar2_center = 64;
  int bar2_width = map(abs(motor2_speed), 0, 255, 0, 45);

  // Draw center line
  display.drawFastVLine(bar2_center, 58, 6, SH110X_WHITE);

  // Draw bar
  if (motor2_speed > 0) {
    // Right side (forward)
    display.fillRect(bar2_center + 1, 59, bar2_width, 4, SH110X_WHITE);
    if (motor2_speed > 0) display.drawChar(bar2_center + bar2_width + 3, 56, '>', SH110X_WHITE, SH110X_BLACK, 1);
  } else if (motor2_speed < 0) {
    // Left side (reverse)
    display.fillRect(bar2_center - bar2_width, 59, bar2_width, 4, SH110X_WHITE);
    if (motor2_speed < 0) display.drawChar(bar2_center - bar2_width - 7, 56, '<', SH110X_WHITE, SH110X_BLACK, 1);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // Wait longer for USB-CDC serial to be ready
  Serial.println("\n\n=== ESP32 Joystick Transmitter Starting ===");
  Serial.flush();

  Serial.println("DEBUG: Setting up LED...");
  Serial.flush();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Start with LED OFF

  // Initialize battery monitoring
  pinMode(VBAT_PIN, INPUT);
  batteryVoltage = readBatteryVoltage();
  Serial.print("Battery voltage: ");
  Serial.print(batteryVoltage);
  Serial.println("V");

  // Check for critically low battery
  if (batteryVoltage < VBAT_CUTOFF && batteryVoltage > 1.0) {
    Serial.println("CRITICAL: Battery too low!");
    while (true) {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }

  #ifdef USE_WIFI
    // WiFi MUST be initialized FIRST, before any other hardware
    Serial.println("Starting WiFi Transmitter...");
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");

    int connect_timeout = 40; // 40 * 500ms = 20 seconds
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      connect_timeout--;
      if (connect_timeout <= 0) {
        Serial.println("\n\nFailed to connect to WiFi! Halting.");
        Serial.println("Please check SSID and password.");
        // Infinite error blink
        while(true) {
          digitalWrite(LED_PIN, LOW);
          delay(150);
          digitalWrite(LED_PIN, HIGH);
          delay(150);
        }
      }
    }

    wifiConnected = true;
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start mDNS responder
    if (MDNS.begin(MDNS_HOSTNAME)) {
      Serial.print("mDNS responder started: ");
      Serial.print(MDNS_HOSTNAME);
      Serial.println(".local");
    } else {
      Serial.println("Error setting up mDNS responder!");
    }

    // Start UDP
    udp.begin(UDP_PORT);
    Serial.print("Ready to send UDP packets on port ");
    Serial.println(UDP_PORT);
  #endif

  // Now initialize other hardware
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);

  // Configure ADC attenuation for better linearity
  analogReadResolution(12); // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db); // 0-3.3V range

  Serial.println("DEBUG: Initializing I2C...");
  Serial.flush();
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized on SDA=GPIO6, SCL=GPIO7");
  Serial.flush();

  Serial.print("Initializing SH1106 display at address 0x");
  Serial.println(SCREEN_ADDRESS, HEX);
  Serial.flush();

  if(!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SH1106 allocation failed!"));
    Serial.flush();
    // Blink error pattern
    while(true) {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }
  Serial.println("DEBUG: Display initialized OK!");
  Serial.flush();
  Serial.println("Display initialized successfully!");

  display.setContrast(255);  // Maximum brightness
  display.clearDisplay();
  display.display();
  delay(100);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);

  #ifdef USE_WIFI
    display.println(F("TX: WiFi OK"));
    display.setCursor(0, 10);
    display.print(F("IP: "));
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
  #else
    display.println(F("TX: BLE Mode"));
    display.display();
    delay(1000);

    BLEDevice::init("ESP32_Joystick_TX");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ   |
      BLECharacteristic::PROPERTY_WRITE  |
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_INDICATE
    );

    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();

    Serial.println("BLE Server Started. Waiting for connection...");
  #endif
}

void loop() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  static unsigned long lastUpdate = 0;

  // Transmitter blinks every 1 second
  if (millis() - lastBlink > 1000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
    Serial.println("TX LED blink (1s interval)");
  }

  // Battery check every 1 second
  static unsigned long lastBatteryCheck = 0;
  if (millis() - lastBatteryCheck > 1000) {
    lastBatteryCheck = millis();
    batteryVoltage = readBatteryVoltage();

    if (batteryVoltage > 1.0 && batteryVoltage < VBAT_WARNING) {
      lowBatteryWarning = true;
    }
  }

  if (millis() - lastUpdate > 100) {
    // Read raw ADC values with oversampling
    int raw_j1x = readADC(JOY1_VRX);
    int raw_j1y = readADC(JOY1_VRY);
    int raw_j2x = readADC(JOY2_VRX);
    int raw_j2y = readADC(JOY2_VRY);

    // Initialize filter on first read
    if (firstRead) {
      filtered_j1x = raw_j1x;
      filtered_j1y = raw_j1y;
      filtered_j2x = raw_j2x;
      filtered_j2y = raw_j2y;
      firstRead = false;
    }

    // Apply exponential moving average filter
    filtered_j1x = (SMOOTHING_ALPHA * raw_j1x) + ((1.0 - SMOOTHING_ALPHA) * filtered_j1x);
    filtered_j1y = (SMOOTHING_ALPHA * raw_j1y) + ((1.0 - SMOOTHING_ALPHA) * filtered_j1y);
    filtered_j2x = (SMOOTHING_ALPHA * raw_j2x) + ((1.0 - SMOOTHING_ALPHA) * filtered_j2x);
    filtered_j2y = (SMOOTHING_ALPHA * raw_j2y) + ((1.0 - SMOOTHING_ALPHA) * filtered_j2y);

    // Store filtered values
    joystickData.joy1_x = (int16_t)filtered_j1x;
    joystickData.joy1_y = (int16_t)filtered_j1y;
    joystickData.joy1_sw = digitalRead(JOY1_SW);

    joystickData.joy2_x = (int16_t)filtered_j2x;
    joystickData.joy2_y = (int16_t)filtered_j2y;
    joystickData.joy2_sw = digitalRead(JOY2_SW);

    Serial.print("J1: X=");
    Serial.print(joystickData.joy1_x);
    Serial.print(" Y=");
    Serial.print(joystickData.joy1_y);
    Serial.print(" | J2: X=");
    Serial.print(joystickData.joy2_x);
    Serial.print(" Y=");
    Serial.println(joystickData.joy2_y);

    String dir1 = getDirection(joystickData.joy1_x, joystickData.joy1_y, JOY1_CENTER_X, JOY1_CENTER_Y);
    String dir2 = getDirection(joystickData.joy2_x, joystickData.joy2_y, JOY2_CENTER_X, JOY2_CENTER_Y);

    display.clearDisplay();

    // Draw joystick visualizations (larger circles for 128x64 display)
    drawJoystick(joystickData.joy1_x, joystickData.joy1_y, joystickData.joy1_sw, 32, 18, 14, "J1", JOY1_CENTER_X, JOY1_CENTER_Y);
    drawJoystick(joystickData.joy2_x, joystickData.joy2_y, joystickData.joy2_sw, 96, 18, 14, "J2", JOY2_CENTER_X, JOY2_CENTER_Y);

    // Draw status indicator
    display.setTextSize(1);
    // Draw battery icon
    drawBatteryIcon(62, 0);

    #ifdef USE_WIFI
      display.setCursor(98, 0);
      if (wifiConnected) {
        display.print(F("WiFi"));
      } else {
        display.print(F("----"));
      }
    #else
      display.setCursor(110, 0);
      if (deviceConnected) {
        display.print(F("TX"));
      } else {
        display.print(F("--"));
      }
    #endif

    // Draw fake servo angle based on Joy1 X-axis
    drawFakeServoAngle(joystickData.joy1_x);

    // Draw fake motor speeds based on Joy2
    drawFakeMotorSpeeds(joystickData.joy2_x, joystickData.joy2_y);

    display.display();

    // Send data via WiFi or BLE
    #ifdef USE_WIFI
      if (wifiConnected) {
        // Send UDP packet to broadcast address
        udp.beginPacket(broadcastIP, UDP_PORT);
        udp.write((uint8_t*)&joystickData, sizeof(joystickData));
        udp.endPacket();
      }
    #else
      if (deviceConnected) {
        pCharacteristic->setValue((uint8_t*)&joystickData, sizeof(joystickData));
        pCharacteristic->notify();
      }
    #endif

    lastUpdate = millis();
  }

  #ifndef USE_WIFI
  // BLE connection management
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("Device connected");
  }
  #endif
}
