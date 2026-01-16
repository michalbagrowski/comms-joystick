#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
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

#define JOY_CENTER_X 1850
#define JOY_CENTER_Y 1850

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

struct JoystickData {
  int16_t joy1_x;
  int16_t joy1_y;
  uint8_t joy1_sw;
  int16_t joy2_x;
  int16_t joy2_y;
  uint8_t joy2_sw;
} joystickData;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

String getDirection(int x, int y) {
  int centerX = 2048;
  int centerY = 2048;
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

void drawJoystick(int16_t joyX, int16_t joyY, uint8_t button, int centerX, int centerY, int radius, const char* label) {
  display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);

  display.drawFastHLine(centerX - radius + 2, centerY, (radius - 2) * 2, SSD1306_WHITE);
  display.drawFastVLine(centerX, centerY - radius + 2, (radius - 2) * 2, SSD1306_WHITE);

  int offsetX = joyX - 2048;
  int offsetY = joyY - 2048;

  int mapX = map(offsetX, -2048, 2047, -radius + 3, radius - 3);
  int mapY = map(offsetY, -2048, 2047, -radius + 3, radius - 3);

  int dotX = centerX + mapX;
  int dotY = centerY + mapY;

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

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.display();
  delay(100);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("TX: Initializing"));
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
}

void loop() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  static unsigned long lastUpdate = 0;

  if (millis() - lastBlink > 500) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }

  if (millis() - lastUpdate > 100) {
    joystickData.joy1_x = analogRead(JOY1_VRX);
    joystickData.joy1_y = analogRead(JOY1_VRY);
    joystickData.joy1_sw = digitalRead(JOY1_SW);

    joystickData.joy2_x = analogRead(JOY2_VRX);
    joystickData.joy2_y = analogRead(JOY2_VRY);
    joystickData.joy2_sw = digitalRead(JOY2_SW);

    Serial.print("J1: X=");
    Serial.print(joystickData.joy1_x);
    Serial.print(" Y=");
    Serial.print(joystickData.joy1_y);
    Serial.print(" | J2: X=");
    Serial.print(joystickData.joy2_x);
    Serial.print(" Y=");
    Serial.println(joystickData.joy2_y);

    String dir1 = getDirection(joystickData.joy1_x, joystickData.joy1_y);
    String dir2 = getDirection(joystickData.joy2_x, joystickData.joy2_y);

    display.clearDisplay();

    drawJoystick(joystickData.joy1_x, joystickData.joy1_y, joystickData.joy1_sw, 32, 18, 14, "J1");
    drawJoystick(joystickData.joy2_x, joystickData.joy2_y, joystickData.joy2_sw, 96, 18, 14, "J2");

    display.setTextSize(1);
    display.setCursor(118, 0);
    if (deviceConnected) {
      display.print(F("TX"));
    } else {
      display.print(F("--"));
    }

    display.display();

    if (deviceConnected) {
      pCharacteristic->setValue((uint8_t*)&joystickData, sizeof(joystickData));
      pCharacteristic->notify();
    }

    lastUpdate = millis();
  }

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
}
