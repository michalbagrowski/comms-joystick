// I2C Scanner for ESP32-C3
// Scans I2C bus and reports found devices

#include <Wire.h>

#define SDA_PIN 6
#define SCL_PIN 7
#define LED_PIN 8

void setup() {
  Serial.begin(115200);
  delay(3000);  // Wait for USB-CDC

  Serial.println("\n\n=== I2C Scanner ===");
  Serial.println("SDA=GPIO6, SCL=GPIO7");
  Serial.flush();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized");
  Serial.flush();
}

void loop() {
  Serial.println("\nScanning I2C bus...");
  Serial.flush();

  int devicesFound = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);

      // Common device identification
      if (address == 0x3C || address == 0x3D) {
        Serial.print(" (OLED SSD1306/SH1106)");
      } else if (address >= 0x20 && address <= 0x27) {
        Serial.print(" (PCF8574 I/O Expander)");
      } else if (address >= 0x48 && address <= 0x4F) {
        Serial.print(" (PCF8591/ADS1115 ADC)");
      } else if (address == 0x68 || address == 0x69) {
        Serial.print(" (MPU6050/DS3231)");
      }
      Serial.println();
      devicesFound++;
    } else if (error == 4) {
      Serial.print("Error at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  Serial.print("\nTotal devices found: ");
  Serial.println(devicesFound);

  if (devicesFound == 0) {
    Serial.println("WARNING: No I2C devices found!");
    Serial.println("Check wiring: SDA->GPIO6, SCL->GPIO7, VCC->3.3V, GND->GND");
  }

  Serial.flush();

  // Blink LED to show we're alive
  digitalWrite(LED_PIN, LOW);
  delay(200);
  digitalWrite(LED_PIN, HIGH);

  delay(5000);  // Scan every 5 seconds
}
