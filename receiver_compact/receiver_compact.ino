// ========================================
// COMPACT RECEIVER - XIAO ESP32-C3 + DRV8833
// ========================================
// Miniaturized build for small chassis (Hot Wheels size)
// - Seeed XIAO ESP32-C3 (21x17mm, built-in LiPo charging)
// - DRV8833 motor driver (10x15mm, nSLEEP replaces transistor)
// - GS-1502 linear servo (3.7-5V, no boost needed)
// - Optional 0.42" OLED (removable for final build)
//
// COMPILE-TIME COMMUNICATION MODE SWITCH
// Uncomment the line below to use WiFi instead of BLE
// #define USE_WIFI

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#ifdef USE_WIFI
  #include <WiFi.h>
  #include <WiFiUdp.h>
  #include <ESPmDNS.h>
#else
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEScan.h>
  #include <BLEAdvertisedDevice.h>
#endif

// ========================================
// DISPLAY CONFIGURATION
// ========================================
// For compact build, can use smaller display or disable entirely
#define USE_DISPLAY  // Comment out to disable display (saves power & space)

#ifdef USE_DISPLAY
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64   // Use 32 for 0.42" OLED, 64 for 0.96"/1.3"
  #define OLED_RESET -1
  #define SCREEN_ADDRESS 0x3C
#endif

// ========================================
// XIAO ESP32-C3 PIN MAPPING
// ========================================
// XIAO pinout (21x17mm board):
//   D0=GPIO2(ADC), D1=GPIO3(ADC), D2=GPIO4(ADC), D3=GPIO5
//   D4=GPIO6(SDA), D5=GPIO7(SCL), D6=GPIO21, D7=GPIO20
//   D8=GPIO8, D9=GPIO9, D10=GPIO10
//
// Note: Only GPIO2-4 (D0-D2) are ADC capable on XIAO!

// I2C for display
#define SDA_PIN 6   // D4 on XIAO
#define SCL_PIN 7   // D5 on XIAO

// Battery monitoring (must use ADC-capable pin)
#define VBAT_PIN 2  // D0 on XIAO (GPIO2, ADC capable)

// DRV8833 Motor Driver pins
// DRV8833 is more efficient than MX1508 and has built-in sleep mode
#define MOTOR1_AIN1 3   // D1 on XIAO - Motor 1 input 1
#define MOTOR1_AIN2 4   // D2 on XIAO - Motor 1 input 2
#define MOTOR2_BIN1 5   // D3 on XIAO - Motor 2 input 1
#define MOTOR2_BIN2 21  // D6 on XIAO - Motor 2 input 2

// DRV8833 nSLEEP pin (LOW = sleep/disabled, HIGH = active)
// This replaces the 2N2222 transistor circuit - much simpler!
#define MOTOR_NSLEEP 20  // D7 on XIAO - Pull HIGH to enable motors

// Servo pin
#define SERVO_PIN 8  // D8 on XIAO

// Status LED
#define LED_PIN 9    // D9 on XIAO (or use built-in LED on GPIO10)

// ========================================
// BATTERY MONITORING (LiPo)
// ========================================
// XIAO ESP32-C3 has built-in LiPo charging via USB-C!
// Still need voltage divider for monitoring:
// LiPo+ --[10K]--+--[10K]-- GND
//                |
//             GPIO2 (D0)
#define VBAT_DIVIDER 2.0
#define VBAT_FULL 4.2             // Fully charged voltage
#define VBAT_EMPTY 3.3            // Empty voltage (0% display threshold)
#define VBAT_WARNING 3.5          // Low battery warning threshold
#define VBAT_CUTOFF 3.2           // Shutdown threshold (protect LiPo)
#define VBAT_SAMPLES 10

// ========================================
// MOTOR CONTROL CONSTANTS
// ========================================
#define MOTOR_DEADZONE 200
#define MOTOR_PWM_FREQ 20000  // 20kHz - inaudible
#define MOTOR_PWM_RES 8       // 8-bit (0-255)

// Joystick center values (from TX calibration)
#define JOY1_CENTER_X 2235
#define JOY1_CENTER_Y 2217
#define JOY2_CENTER_X 2215
#define JOY2_CENTER_Y 2255

// ========================================
// COMMUNICATION CONFIGURATION
// ========================================
#ifdef USE_WIFI
  #define WIFI_SSID "amplifi"
  #define WIFI_PASSWORD "123qwe123"
  #define UDP_PORT 4210
  #define MDNS_HOSTNAME "esp32-joystick-tx"
#else
  #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
  #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#endif

// ========================================
// GLOBAL OBJECTS
// ========================================
#ifdef USE_DISPLAY
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

Servo myServo;

// Motor/servo state
int motor1_speed = 0;
int motor2_speed = 0;
int servo_angle = 90;

// Communication globals
#ifdef USE_WIFI
  WiFiUDP udp;
  IPAddress transmitterIP;
  volatile bool wifiConnected = false;
  volatile bool transmitterFound = false;
  volatile bool dataReceived = false;
#else
  static boolean doConnect = false;
  static boolean connected = false;
  static boolean doScan = false;
  static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
  static BLEAdvertisedDevice* myDevice = nullptr;
#endif

struct JoystickData {
  int16_t joy1_x;
  int16_t joy1_y;
  uint8_t joy1_sw;
  int16_t joy2_x;
  int16_t joy2_y;
  uint8_t joy2_sw;
} joystickData = {
  JOY1_CENTER_X, JOY1_CENTER_Y, HIGH,
  JOY2_CENTER_X, JOY2_CENTER_Y, HIGH
};

volatile bool outputsEnabled = false;
bool servoAttached = false;

// Communication timeout - stop motors if no data received
#define COMM_TIMEOUT_MS 500
volatile unsigned long lastDataTime = 0;

// Battery state
float batteryVoltage = 4.2;
bool lowBatteryWarning = false;
bool batteryShutdown = false;

// ========================================
// BATTERY FUNCTIONS
// ========================================
float readBatteryVoltage() {
  long sum = 0;
  for (int i = 0; i < VBAT_SAMPLES; i++) {
    sum += analogRead(VBAT_PIN);
    delayMicroseconds(100);
  }
  float avgRaw = (float)sum / VBAT_SAMPLES;
  return (avgRaw / 4095.0) * 3.3 * VBAT_DIVIDER;
}

int getBatteryPercent() {
  if (batteryVoltage >= VBAT_FULL) return 100;
  if (batteryVoltage <= VBAT_EMPTY) return 0;
  return (int)((batteryVoltage - VBAT_EMPTY) / (VBAT_FULL - VBAT_EMPTY) * 100);
}

#ifdef USE_DISPLAY
void drawBatteryIcon(int x, int y) {
  int percent = getBatteryPercent();
  display.drawRect(x, y, 14, 8, SSD1306_WHITE);
  display.fillRect(x + 14, y + 2, 2, 4, SSD1306_WHITE);
  // Fill level (0-3 bars): maps 0-100% to 0-3 with rounding
  // Formula: (percent + 16) / 33 distributes evenly: 0-16%=0, 17-49%=1, 50-82%=2, 83-100%=3
  int bars = (percent + 16) / 33;
  if (bars > 0) display.fillRect(x + 2, y + 2, 3, 4, SSD1306_WHITE);
  if (bars > 1) display.fillRect(x + 6, y + 2, 3, 4, SSD1306_WHITE);
  if (bars > 2) display.fillRect(x + 10, y + 2, 2, 4, SSD1306_WHITE);
  // Blink warning: draw outer border that flashes (doesn't obscure level bars)
  if (lowBatteryWarning && (millis() / 500) % 2 == 0) {
    display.drawRect(x - 1, y - 1, 16, 10, SSD1306_WHITE);
  }
}
#endif

// ========================================
// DRV8833 MOTOR CONTROL
// ========================================
// DRV8833 control modes:
// AIN1=PWM, AIN2=LOW  -> Forward at PWM speed
// AIN1=LOW, AIN2=PWM  -> Reverse at PWM speed
// AIN1=LOW, AIN2=LOW  -> Coast (free spin)
// AIN1=HIGH, AIN2=HIGH -> Brake (slow decay)

void controlMotors(int16_t joy2_x, int16_t joy2_y) {
  if (batteryShutdown) {
    // Emergency stop - coast mode
    ledcWrite(MOTOR1_AIN1, 0);
    ledcWrite(MOTOR1_AIN2, 0);
    ledcWrite(MOTOR2_BIN1, 0);
    ledcWrite(MOTOR2_BIN2, 0);
    motor1_speed = 0;
    motor2_speed = 0;
    return;
  }

  // Motor 1 from Joy2 X-axis (left/right)
  int offset_x = joy2_x - JOY2_CENTER_X;
  if (abs(offset_x) < MOTOR_DEADZONE) {
    motor1_speed = 0;
  } else {
    motor1_speed = map(offset_x, -JOY2_CENTER_X, 4095 - JOY2_CENTER_X, -255, 255);
    motor1_speed = constrain(motor1_speed, -255, 255);
  }

  // Motor 2 from Joy2 Y-axis (forward/backward)
  int offset_y = joy2_y - JOY2_CENTER_Y;
  if (abs(offset_y) < MOTOR_DEADZONE) {
    motor2_speed = 0;
  } else {
    motor2_speed = map(offset_y, -JOY2_CENTER_Y, 4095 - JOY2_CENTER_Y, -255, 255);
    motor2_speed = constrain(motor2_speed, -255, 255);
  }

  // Apply Motor 1 (using pin numbers with new API)
  if (motor1_speed > 0) {
    ledcWrite(MOTOR1_AIN1, motor1_speed);  // AIN1 = PWM (forward)
    ledcWrite(MOTOR1_AIN2, 0);             // AIN2 = LOW
  } else if (motor1_speed < 0) {
    ledcWrite(MOTOR1_AIN1, 0);             // AIN1 = LOW
    ledcWrite(MOTOR1_AIN2, -motor1_speed); // AIN2 = PWM (reverse)
  } else {
    ledcWrite(MOTOR1_AIN1, 0);  // Coast
    ledcWrite(MOTOR1_AIN2, 0);
  }

  // Apply Motor 2
  if (motor2_speed > 0) {
    ledcWrite(MOTOR2_BIN1, motor2_speed);  // BIN1 = PWM (forward)
    ledcWrite(MOTOR2_BIN2, 0);             // BIN2 = LOW
  } else if (motor2_speed < 0) {
    ledcWrite(MOTOR2_BIN1, 0);             // BIN1 = LOW
    ledcWrite(MOTOR2_BIN2, -motor2_speed); // BIN2 = PWM (reverse)
  } else {
    ledcWrite(MOTOR2_BIN1, 0);  // Coast
    ledcWrite(MOTOR2_BIN2, 0);
  }
}

void enableMotors(bool enable) {
  digitalWrite(MOTOR_NSLEEP, enable ? HIGH : LOW);
}

// ========================================
// DISPLAY FUNCTIONS
// ========================================
#ifdef USE_DISPLAY
void drawJoystick(int joyX, int joyY, uint8_t sw, int centerX, int centerY, int radius, const char* label, int joyCenterX, int joyCenterY) {
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(centerX - 6, centerY - radius - 10);
  display.print(label);

  int zeroX = centerX + (radius - 3);
  int zeroY = centerY + (radius - 3);
  display.drawLine(zeroX - 2, zeroY, zeroX + 2, zeroY, SSD1306_WHITE);
  display.drawLine(zeroX, zeroY - 2, zeroX, zeroY + 2, SSD1306_WHITE);

  int offsetX = joyX - joyCenterX;
  int offsetY = joyY - joyCenterY;
  int mapX = map(offsetX, -joyCenterX, 4095 - joyCenterX, -2*(radius - 3), 0);
  int mapY = map(offsetY, -joyCenterY, 4095 - joyCenterY, -2*(radius - 3), 0);
  int dotX = constrain(zeroX + mapX, centerX - radius + 2, centerX + radius - 2);
  int dotY = constrain(zeroY + mapY, centerY - radius + 2, centerY + radius - 2);

  if (sw == LOW) {
    display.fillCircle(dotX, dotY, 3, SSD1306_WHITE);
  } else {
    display.drawCircle(dotX, dotY, 2, SSD1306_WHITE);
  }
}

void drawServoBar(int angle) {
  int barY = 36;
  int barWidth = map(angle, 0, 180, 0, 100);
  display.fillRect(14, barY, barWidth, 6, SSD1306_WHITE);
  display.drawRect(14, barY, 100, 6, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, barY);
  display.print("S");
  display.setCursor(116, barY);
  display.print(angle);
}

void drawMotorBars(int m1, int m2) {
  int barY1 = 48;
  int barY2 = 56;
  int centerX = 64;

  // Motor 1 bar
  display.setCursor(0, barY1);
  display.print("M1");
  display.drawRect(14, barY1, 100, 6, SSD1306_WHITE);
  if (m1 > 0) {
    int width = map(abs(m1), 0, 255, 0, 50);
    display.fillRect(centerX, barY1, width, 6, SSD1306_WHITE);
  } else if (m1 < 0) {
    int width = map(abs(m1), 0, 255, 0, 50);
    display.fillRect(centerX - width, barY1, width, 6, SSD1306_WHITE);
  }
  display.drawLine(centerX, barY1, centerX, barY1 + 5, SSD1306_WHITE);

  // Motor 2 bar
  display.setCursor(0, barY2);
  display.print("M2");
  display.drawRect(14, barY2, 100, 6, SSD1306_WHITE);
  if (m2 > 0) {
    int width = map(abs(m2), 0, 255, 0, 50);
    display.fillRect(centerX, barY2, width, 6, SSD1306_WHITE);
  } else if (m2 < 0) {
    int width = map(abs(m2), 0, 255, 0, 50);
    display.fillRect(centerX - width, barY2, width, 6, SSD1306_WHITE);
  }
  display.drawLine(centerX, barY2, centerX, barY2 + 5, SSD1306_WHITE);
}
#endif

// ========================================
// BLE CALLBACKS (when not using WiFi)
// ========================================
#ifndef USE_WIFI
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("BLE connected");
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    outputsEnabled = false;
    Serial.println("BLE disconnected");
  }
};

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      // Free previous device if exists to prevent memory leak
      if (myDevice != nullptr) {
        delete myDevice;
      }
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;
    }
  }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData, size_t length, bool isNotify) {
  if (length == sizeof(JoystickData)) {
    memcpy(&joystickData, pData, sizeof(JoystickData));
    lastDataTime = millis();  // Update timestamp for timeout detection
    if (!outputsEnabled) {
      outputsEnabled = true;
      enableMotors(true);
    }
  }
}

bool connectToServer() {
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
  pClient->connect(myDevice);

  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
  if (pRemoteCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }

  connected = true;
  return true;
}
#endif

// ========================================
// SETUP
// ========================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== Compact RX (XIAO + DRV8833) ===");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED on during init

  // Initialize motor driver sleep pin FIRST (keep motors disabled)
  pinMode(MOTOR_NSLEEP, OUTPUT);
  digitalWrite(MOTOR_NSLEEP, LOW);  // Motors disabled (sleep mode)

  // Initialize motor PWM (ESP32 Arduino Core 3.x API)
  // ledcAttach(pin, freq, resolution) replaces ledcSetup + ledcAttachPin
  ledcAttach(MOTOR1_AIN1, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR1_AIN2, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR2_BIN1, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR2_BIN2, MOTOR_PWM_FREQ, MOTOR_PWM_RES);

  // Ensure all motor outputs are LOW
  ledcWrite(MOTOR1_AIN1, 0);
  ledcWrite(MOTOR1_AIN2, 0);
  ledcWrite(MOTOR2_BIN1, 0);
  ledcWrite(MOTOR2_BIN2, 0);

  delay(50);  // Stabilization delay

  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(90);  // Center position
  delay(100);

  // Initialize I2C and display
  Wire.begin(SDA_PIN, SCL_PIN);

  #ifdef USE_DISPLAY
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println("Display init failed");
    } else {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Compact RX");
      display.println("XIAO + DRV8833");
      display.display();
    }
  #endif

  // Check battery voltage
  batteryVoltage = readBatteryVoltage();
  Serial.print("Battery: ");
  Serial.print(batteryVoltage);
  Serial.println("V");

  if (batteryVoltage > 1.0 && batteryVoltage < 3.2) {
    Serial.println("CRITICAL: Battery too low!");
    #ifdef USE_DISPLAY
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 20);
      display.println("BATTERY");
      display.println("CRITICAL");
      display.display();
    #endif
    while (1) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(200);
    }
  }

  // Initialize communication
  #ifdef USE_WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(500);
      Serial.print(".");
      timeout++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
      udp.begin(UDP_PORT);
    } else {
      Serial.println("\nWiFi failed");
    }
  #else
    BLEDevice::init("Compact-RX");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
    Serial.println("BLE scanning...");
  #endif

  digitalWrite(LED_PIN, LOW);  // LED off, init complete
  Serial.println("Setup complete");
}

// ========================================
// MAIN LOOP
// ========================================
void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastLedToggle = 0;
  static unsigned long lastBatteryCheck = 0;

  // LED blink (3 second interval)
  if (millis() - lastLedToggle > 3000) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastLedToggle = millis();
  }

  // Battery check every second
  if (millis() - lastBatteryCheck > 1000) {
    lastBatteryCheck = millis();
    batteryVoltage = readBatteryVoltage();

    if (batteryVoltage > 1.0) {  // Valid reading (battery connected)
      if (batteryVoltage < VBAT_WARNING) {
        lowBatteryWarning = true;
      }
      if (batteryVoltage < VBAT_CUTOFF && !batteryShutdown) {
        batteryShutdown = true;
        enableMotors(false);
        Serial.println("LOW BATTERY SHUTDOWN");
      }
    }
  }

  // Communication timeout failsafe - stop motors if no data received
  if (outputsEnabled && (millis() - lastDataTime > COMM_TIMEOUT_MS)) {
    Serial.println("FAILSAFE: Communication timeout - stopping motors");
    enableMotors(false);
    motor1_speed = 0;
    motor2_speed = 0;
    outputsEnabled = false;  // Will re-enable when data resumes
  }

  // Handle communication
  #ifdef USE_WIFI
    if (wifiConnected) {
      int packetSize = udp.parsePacket();
      if (packetSize == sizeof(JoystickData)) {
        udp.read((uint8_t*)&joystickData, sizeof(JoystickData));
        lastDataTime = millis();  // Update timestamp for timeout detection
        if (!outputsEnabled) {
          outputsEnabled = true;
          enableMotors(true);
        }
        dataReceived = true;
      }
    }
  #else
    if (doConnect) {
      if (connectToServer()) {
        Serial.println("Connected to TX");
      } else {
        Serial.println("Connection failed");
      }
      doConnect = false;
    }

    if (!connected && !doConnect) {
      BLEDevice::getScan()->start(0);
    }
  #endif

  // Update motors, servo, and display at 10Hz
  if (millis() - lastUpdate > 100) {
    lastUpdate = millis();

    if (outputsEnabled && !batteryShutdown) {
      // Control servo from Joy1 X-axis
      servo_angle = map(joystickData.joy1_x, 0, 4095, 0, 180);
      servo_angle = constrain(servo_angle, 0, 180);
      myServo.write(servo_angle);

      // Control motors from Joy2
      controlMotors(joystickData.joy2_x, joystickData.joy2_y);
    }

    #ifdef USE_DISPLAY
      display.clearDisplay();

      // Draw joystick positions
      drawJoystick(joystickData.joy1_x, joystickData.joy1_y, joystickData.joy1_sw,
                   25, 14, 10, "J1", JOY1_CENTER_X, JOY1_CENTER_Y);
      drawJoystick(joystickData.joy2_x, joystickData.joy2_y, joystickData.joy2_sw,
                   75, 14, 10, "J2", JOY2_CENTER_X, JOY2_CENTER_Y);

      // Battery icon
      drawBatteryIcon(100, 0);

      // Status indicator
      display.setTextSize(1);
      #ifdef USE_WIFI
        display.setCursor(100, 10);
        display.print(wifiConnected ? "WiFi" : "----");
      #else
        display.setCursor(108, 10);
        display.print(connected ? "RX" : "--");
      #endif

      // Servo and motor bars
      drawServoBar(servo_angle);
      drawMotorBars(motor1_speed, motor2_speed);

      // Low battery warning overlay
      if (batteryShutdown) {
        display.fillRect(20, 25, 88, 15, SSD1306_BLACK);
        display.drawRect(20, 25, 88, 15, SSD1306_WHITE);
        display.setCursor(24, 29);
        display.print("LOW BATTERY!");
      }

      display.display();
    #endif
  }
}
