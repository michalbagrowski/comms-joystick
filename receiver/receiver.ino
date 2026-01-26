// ========================================
// COMPILE-TIME COMMUNICATION MODE SWITCH
// ========================================
// Uncomment the line below to use WiFi instead of BLE
// #define USE_WIFI  // Comment out to use BLE mode

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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 6
#define SCL_PIN 7

// Servo control pin
#define SERVO_PIN 4  // Servo controlled by Joy1 X-axis

// Motor control pins (MX1508 driver)
#define MOTOR1_IN1 0  // Motor 1 forward (Joy2 X-axis)
#define MOTOR1_IN2 1  // Motor 1 reverse
#define MOTOR2_IN3 2  // Motor 2 forward (Joy2 Y-axis)
#define MOTOR2_IN4 3  // Motor 2 reverse

#define LED_PIN 8    // Built-in LED for error indication

// Motor enable pin - controls power to MX1508 via transistor
// This prevents motor twitch during boot (MX1508 has no power until code enables it)
#define MOTOR_ENABLE_PIN 5

// Motor control constants
#define MOTOR_DEADZONE 200   // Deadzone around center to prevent motor jitter
#define MOTOR_PWM_FREQ 20000 // 20kHz PWM frequency for motors
#define MOTOR_PWM_RES 8      // 8-bit resolution (0-255)

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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo myServo;  // Create servo object

// Motor speed variables (global to track current speeds)
int motor1_speed = 0;
int motor2_speed = 0;
int servo_angle = 90;  // Current servo angle

// ========================================
// COMMUNICATION GLOBALS
// ========================================
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
  static BLERemoteCharacteristic* pRemoteCharacteristic;
  static BLEAdvertisedDevice* myDevice;
#endif

struct JoystickData {
  int16_t joy1_x;
  int16_t joy1_y;
  uint8_t joy1_sw;
  int16_t joy2_x;
  int16_t joy2_y;
  uint8_t joy2_sw;
} joystickData = {
  JOY1_CENTER_X, JOY1_CENTER_Y, HIGH,  // Initialize to center (motors stop)
  JOY2_CENTER_X, JOY2_CENTER_Y, HIGH
};

// Flag to prevent motor/servo activation until valid data received
volatile bool outputsEnabled = false;  // volatile for BLE callback safety
bool servoAttached = false;

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
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);

  // Zero state position (bottom-right corner of circle)
  int zeroX = centerX + (radius - 3);
  int zeroY = centerY + (radius - 3);

  // Draw crosshairs at zero state position
  display.drawFastHLine(zeroX - 4, zeroY, 8, SSD1306_WHITE);
  display.drawFastVLine(zeroX, zeroY - 4, 8, SSD1306_WHITE);

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
    display.fillCircle(dotX, dotY, 2, SSD1306_WHITE);
  } else {
    display.drawCircle(dotX, dotY, 2, SSD1306_WHITE);
  }

  display.setTextSize(1);
  int labelX = centerX - 6;
  int labelY = 0;
  display.setCursor(labelX, labelY);
  display.print(label);
}

// Motor control function
void controlMotors(int16_t joy2_x, int16_t joy2_y) {
  // Calculate motor 1 speed from Joy2 X-axis (left/right)
  int joy2x_offset = joy2_x - JOY2_CENTER_X;
  if (abs(joy2x_offset) < MOTOR_DEADZONE) {
    motor1_speed = 0;
  } else {
    motor1_speed = map(joy2_x, 0, 4095, -255, 255);
    motor1_speed = constrain(motor1_speed, -255, 255);
  }

  // Calculate motor 2 speed from Joy2 Y-axis (forward/backward)
  int joy2y_offset = joy2_y - JOY2_CENTER_Y;
  if (abs(joy2y_offset) < MOTOR_DEADZONE) {
    motor2_speed = 0;
  } else {
    motor2_speed = map(joy2_y, 0, 4095, -255, 255);
    motor2_speed = constrain(motor2_speed, -255, 255);
  }

  // Control Motor 1 (MX1508 bidirectional PWM)
  // New API: ledcWrite(pin, dutyCycle) - write directly to pin
  if (motor1_speed > 0) {
    ledcWrite(MOTOR1_IN1, motor1_speed);  // Forward
    ledcWrite(MOTOR1_IN2, 0);
  } else if (motor1_speed < 0) {
    ledcWrite(MOTOR1_IN1, 0);
    ledcWrite(MOTOR1_IN2, -motor1_speed); // Reverse
  } else {
    ledcWrite(MOTOR1_IN1, 0);
    ledcWrite(MOTOR1_IN2, 0);
  }

  // Control Motor 2 (MX1508 bidirectional PWM)
  if (motor2_speed > 0) {
    ledcWrite(MOTOR2_IN3, motor2_speed);  // Forward
    ledcWrite(MOTOR2_IN4, 0);
  } else if (motor2_speed < 0) {
    ledcWrite(MOTOR2_IN3, 0);
    ledcWrite(MOTOR2_IN4, -motor2_speed); // Reverse
  } else {
    ledcWrite(MOTOR2_IN3, 0);
    ledcWrite(MOTOR2_IN4, 0);
  }
}

// Draw servo position bar
void drawServoBar() {
  // Servo angle text
  display.setCursor(0, 32);
  display.setTextSize(1);
  display.print("S:");
  display.print(servo_angle);
  display.print((char)247);  // Degree symbol

  // Servo position bar (horizontal, fills left-to-right)
  int barY = 40;
  display.drawRect(24, barY, 100, 6, SSD1306_WHITE);
  int barWidth = map(servo_angle, 0, 180, 2, 98);
  display.fillRect(25, barY + 1, barWidth, 4, SSD1306_WHITE);
}

// Draw motor speed bars (graphical representation)
void drawMotorBars() {
  // Motor 1 bar (horizontal, centered at y=50)
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.print("M1:");

  int bar1_center = 64;
  int bar1_width = map(abs(motor1_speed), 0, 255, 0, 45);

  // Draw center line
  display.drawFastVLine(bar1_center, 50, 6, SSD1306_WHITE);

  // Draw bar
  if (motor1_speed > 0) {
    // Right side (forward)
    display.fillRect(bar1_center + 1, 51, bar1_width, 4, SSD1306_WHITE);
    if (motor1_speed > 0) display.drawChar(bar1_center + bar1_width + 3, 48, '>', SSD1306_WHITE, SSD1306_BLACK, 1);
  } else if (motor1_speed < 0) {
    // Left side (reverse)
    display.fillRect(bar1_center - bar1_width, 51, bar1_width, 4, SSD1306_WHITE);
    if (motor1_speed < 0) display.drawChar(bar1_center - bar1_width - 7, 48, '<', SSD1306_WHITE, SSD1306_BLACK, 1);
  }

  // Motor 2 bar (horizontal, centered at y=58)
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print("M2:");

  int bar2_center = 64;
  int bar2_width = map(abs(motor2_speed), 0, 255, 0, 45);

  // Draw center line
  display.drawFastVLine(bar2_center, 58, 6, SSD1306_WHITE);

  // Draw bar
  if (motor2_speed > 0) {
    // Right side (forward)
    display.fillRect(bar2_center + 1, 59, bar2_width, 4, SSD1306_WHITE);
    if (motor2_speed > 0) display.drawChar(bar2_center + bar2_width + 3, 56, '>', SSD1306_WHITE, SSD1306_BLACK, 1);
  } else if (motor2_speed < 0) {
    // Left side (reverse)
    display.fillRect(bar2_center - bar2_width, 59, bar2_width, 4, SSD1306_WHITE);
    if (motor2_speed < 0) display.drawChar(bar2_center - bar2_width - 7, 56, '<', SSD1306_WHITE, SSD1306_BLACK, 1);
  }
}

// ========================================
// COMMUNICATION CALLBACKS & FUNCTIONS
// ========================================
#ifndef USE_WIFI
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  if (length == sizeof(joystickData)) {
    memcpy(&joystickData, pData, sizeof(joystickData));
    outputsEnabled = true;  // Enable outputs after first valid data
  }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  pClient->connect(myDevice);
  Serial.println(" - Connected to server");

  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(SERVICE_UUID);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(CHARACTERISTIC_UUID);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  if(pRemoteCharacteristic->canRead()) {
    String value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value);
  }

  if(pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
    }
  }
};
#endif

void setup() {
  // CRITICAL: Disable motor driver power FIRST via enable pin
  // This cuts power to MX1508 so motors can't move during boot
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  digitalWrite(MOTOR_ENABLE_PIN, LOW);  // MX1508 has no GND = no power

  // Set motor control pins LOW as well (belt and suspenders)
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN3, OUTPUT);
  pinMode(MOTOR2_IN4, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(MOTOR1_IN1, LOW);
  digitalWrite(MOTOR1_IN2, LOW);
  digitalWrite(MOTOR2_IN3, LOW);
  digitalWrite(MOTOR2_IN4, LOW);
  digitalWrite(SERVO_PIN, LOW);

  // Small delay to let pins stabilize before any other init
  delay(50);

  Serial.begin(115200);
  delay(2000);  // Wait longer for USB-CDC serial to be ready
  Serial.println("\n\n=== ESP32 Joystick Receiver Starting ===");
  Serial.flush();

  Serial.println("DEBUG: Setting up LED...");
  Serial.flush();
  // Initialize LED pin for status blinking
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Start with LED OFF (active LOW)

  #ifdef USE_WIFI
    // WiFi MUST be initialized FIRST, before any other hardware
    Serial.println("Starting WiFi Receiver...");
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
        // Infinite error indication - blink LED since servo not initialized yet
        pinMode(LED_PIN, OUTPUT);
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

    // Start UDP listener
    udp.begin(UDP_PORT);
    Serial.print("Listening for UDP packets on port ");
    Serial.println(UDP_PORT);

    // Initialize mDNS for discovery
    if (!MDNS.begin("esp32-joystick-rx")) {
      Serial.println("Error setting up mDNS!");
    }

    // Discover transmitter via mDNS
    Serial.print("Discovering transmitter via mDNS: ");
    Serial.print(MDNS_HOSTNAME);
    Serial.println(".local");

    int discoveryAttempts = 0;
    while (!transmitterFound && discoveryAttempts < 20) {
      transmitterIP = MDNS.queryHost(MDNS_HOSTNAME);
      if (transmitterIP != IPAddress(0, 0, 0, 0)) {
        transmitterFound = true;
        Serial.print("Transmitter found at: ");
        Serial.println(transmitterIP);
      } else {
        Serial.print(".");
        delay(500);
        discoveryAttempts++;
      }
    }

    if (!transmitterFound) {
      Serial.println("\nTransmitter discovery failed!");
      Serial.println("Will listen for broadcast packets...");
    }
  #else
    Serial.println("Starting Arduino BLE Client application...");
  #endif

  // Small delay before servo attach to let things stabilize
  delay(100);

  // Attach servo and set to center position
  myServo.attach(SERVO_PIN);
  myServo.write(90);  // Start at center
  servoAttached = true;
  Serial.println("Servo initialized at 90 degrees on GPIO4");

  // Initialize motor PWM (ESP32 LEDC - new API for core 3.x)
  // New API: ledcAttach(pin, freq, resolution) - no channels needed!
  ledcAttach(MOTOR1_IN1, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR1_IN2, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR2_IN3, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
  ledcAttach(MOTOR2_IN4, MOTOR_PWM_FREQ, MOTOR_PWM_RES);

  // Small delay to let PWM stabilize
  delay(10);

  // Stop motors explicitly (multiple times to ensure)
  ledcWrite(MOTOR1_IN1, 0);
  ledcWrite(MOTOR1_IN2, 0);
  ledcWrite(MOTOR2_IN3, 0);
  ledcWrite(MOTOR2_IN4, 0);
  delay(10);
  ledcWrite(MOTOR1_IN1, 0);
  ledcWrite(MOTOR1_IN2, 0);
  ledcWrite(MOTOR2_IN3, 0);
  ledcWrite(MOTOR2_IN4, 0);

  Serial.println("Motors initialized (MX1508 driver)");
  Serial.flush();

  Serial.println("DEBUG: Initializing I2C...");
  Serial.flush();
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized on SDA=GPIO6, SCL=GPIO7");
  Serial.flush();

  Serial.print("DEBUG: Initializing SH1106 display at address 0x");
  Serial.println(SCREEN_ADDRESS, HEX);
  Serial.flush();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SH1106 allocation failed"));
    Serial.flush();
    for(;;);
  }
  Serial.println("DEBUG: Display initialized OK!");
  Serial.flush();

  // Set display brightness to maximum
  // Set max brightness
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0xFF);

  display.clearDisplay();
  display.display();
  delay(100);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  #ifdef USE_WIFI
    display.println(F("RX: WiFi OK"));
    display.setCursor(0, 10);
    display.print(F("IP: "));
    display.println(WiFi.localIP());
    if (transmitterFound) {
      display.setCursor(0, 20);
      display.print(F("TX: "));
      display.println(transmitterIP);
    }
    display.display();
    delay(2000);
  #else
    display.println(F("RX: BLE Mode"));
    display.display();
    delay(1000);

    BLEDevice::init("ESP32_Joystick_RX");

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
  #endif

  // NOW enable motor driver power - all pins are configured correctly
  Serial.println("Enabling motor driver...");
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);  // MX1508 now has GND = powered
  Serial.println("Motor driver enabled. Setup complete.");
}

void loop() {
  // Receiver blinks every 3 seconds
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  if (millis() - lastBlink > 3000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
    Serial.println("RX LED blink (3s interval)");
  }

  #ifdef USE_WIFI
    // WiFi mode: receive UDP packets
    if (wifiConnected) {
      int packetSize = udp.parsePacket();
      if (packetSize > 0) {
        if (packetSize == sizeof(joystickData)) {
          udp.read((uint8_t*)&joystickData, sizeof(joystickData));
          dataReceived = true;
          outputsEnabled = true;  // Enable outputs after first valid data

          // Store transmitter IP if we didn't find it via mDNS
          if (!transmitterFound) {
            transmitterIP = udp.remoteIP();
            transmitterFound = true;
            Serial.print("Transmitter found at: ");
            Serial.println(transmitterIP);
          }
        } else {
          Serial.print("Invalid packet size: ");
          Serial.println(packetSize);
        }
      }

      if (dataReceived) {
        // Only control outputs after valid data received
        if (outputsEnabled) {
          // Control servo based on Joystick 1 X-axis
          servo_angle = map(joystickData.joy1_x, 0, 4095, 0, 180);
          servo_angle = constrain(servo_angle, 0, 180);
          myServo.write(servo_angle);

          // Control motors based on Joystick 2
          controlMotors(joystickData.joy2_x, joystickData.joy2_y);
        }

        display.clearDisplay();

        // Draw joystick visualizations (larger circles for 128x64 display)
        drawJoystick(joystickData.joy1_x, joystickData.joy1_y, joystickData.joy1_sw, 32, 18, 14, "J1", JOY1_CENTER_X, JOY1_CENTER_Y);
        drawJoystick(joystickData.joy2_x, joystickData.joy2_y, joystickData.joy2_sw, 96, 18, 14, "J2", JOY2_CENTER_X, JOY2_CENTER_Y);

        // Draw status indicator
        display.setTextSize(1);
        display.setCursor(98, 0);
        display.print(F("WiFi"));

        // Draw servo position bar
        drawServoBar();

        // Draw motor speed bars
        drawMotorBars();

        display.display();
      } else {
        // Waiting for data
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("RX: Waiting..."));
        display.display();
      }
    } else {
      // WiFi not connected
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("RX: No WiFi"));
      display.display();
    }

  #else
    // BLE mode: existing BLE logic
    if (doConnect == true) {
      if (connectToServer()) {
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothing more we will do.");
      }
      doConnect = false;
    }

    if (connected) {
      // Only control outputs after valid data received
      if (outputsEnabled) {
        // Control servo based on Joystick 1 X-axis
        servo_angle = map(joystickData.joy1_x, 0, 4095, 0, 180);
        servo_angle = constrain(servo_angle, 0, 180);
        myServo.write(servo_angle);

        // Control motors based on Joystick 2
        controlMotors(joystickData.joy2_x, joystickData.joy2_y);
      }

      display.clearDisplay();

      // Draw joystick visualizations (larger circles for 128x64 display)
      drawJoystick(joystickData.joy1_x, joystickData.joy1_y, joystickData.joy1_sw, 32, 18, 14, "J1", JOY1_CENTER_X, JOY1_CENTER_Y);
      drawJoystick(joystickData.joy2_x, joystickData.joy2_y, joystickData.joy2_sw, 96, 18, 14, "J2", JOY2_CENTER_X, JOY2_CENTER_Y);

      // Draw status indicator
      display.setTextSize(1);
      display.setCursor(110, 0);
      display.print(F("RX"));

      // Draw servo position bar
      drawServoBar();

      // Draw motor speed bars
      drawMotorBars();

      display.display();
    } else if (doScan) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("RX: Scanning..."));
      display.display();
      BLEDevice::getScan()->start(0);
    }
  #endif

  delay(100);
}
