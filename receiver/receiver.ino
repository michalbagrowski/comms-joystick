#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ESP32Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 6
#define SCL_PIN 7
#define SERVO_PIN 0  // GPIO0 for servo control

// Joystick 1 center values (measured at rest)
#define JOY1_CENTER_X 3515
#define JOY1_CENTER_Y 3234

// Joystick 2 center values (measured at rest)
#define JOY2_CENTER_X 3352
#define JOY2_CENTER_Y 3510

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo myServo;  // Create servo object

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

struct JoystickData {
  int16_t joy1_x;
  int16_t joy1_y;
  uint8_t joy1_sw;
  int16_t joy2_x;
  int16_t joy2_y;
  uint8_t joy2_sw;
} joystickData;

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

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  if (length == sizeof(joystickData)) {
    memcpy(&joystickData, pData, sizeof(joystickData));
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

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

  // Initialize servo
  myServo.attach(SERVO_PIN);  // Attach servo to GPIO0
  myServo.write(90);          // Start at center position (90 degrees)
  Serial.println("Servo initialized at 90 degrees");

  Wire.begin(SDA_PIN, SCL_PIN);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Set display brightness/contrast (0-255, default is 127)
  // Higher = brighter. 255 = maximum brightness
  display.ssd1306_command(0x81); // Set contrast control
  display.ssd1306_command(0xFF); // Maximum brightness (255)

  display.clearDisplay();
  display.display();
  delay(100);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("RX: Initializing"));
  display.display();
  delay(1000);

  BLEDevice::init("ESP32_Joystick_RX");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  if (connected) {
    // Control servo based on Joystick 1 X-axis
    // Map joystick value (0-4095) to servo angle (0-180 degrees)
    int servoAngle = map(joystickData.joy1_x, 0, 4095, 0, 180);
    servoAngle = constrain(servoAngle, 0, 180);  // Ensure within valid range
    myServo.write(servoAngle);

    display.clearDisplay();

    // Draw joystick visualizations (smaller circles for compact display)
    drawJoystick(joystickData.joy1_x, joystickData.joy1_y, joystickData.joy1_sw, 25, 10, 8, "J1", JOY1_CENTER_X, JOY1_CENTER_Y);
    drawJoystick(joystickData.joy2_x, joystickData.joy2_y, joystickData.joy2_sw, 75, 10, 8, "J2", JOY2_CENTER_X, JOY2_CENTER_Y);

    // Draw status indicator
    display.setTextSize(1);
    display.setCursor(110, 0);
    display.print(F("RX"));

    // Draw servo position bar at bottom
    // Bar from x=0 to x=127, height 4 pixels at y=28-31
    display.drawRect(0, 28, 128, 4, SSD1306_WHITE);  // Outer frame

    // Fill bar based on servo angle (0-180 maps to 2-126 pixels)
    int barWidth = map(servoAngle, 0, 180, 2, 126);
    display.fillRect(1, 29, barWidth, 2, SSD1306_WHITE);

    // Draw servo angle text above bar
    display.setCursor(0, 20);
    display.print("S:");
    display.print(servoAngle);
    display.print((char)247);  // Degree symbol

    display.display();
  } else if (doScan) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("RX: Scanning..."));
    display.display();
    BLEDevice::getScan()->start(0);
  }

  delay(100);
}
