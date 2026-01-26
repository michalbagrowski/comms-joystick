// Display Test - Try BOTH SSD1306 and SH1106
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  // Try SSD1306 first

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 6
#define SCL_PIN 7
#define LED_PIN 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=== Display Test (SSD1306) ===");
  Serial.flush();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Initializing I2C on GPIO6/GPIO7...");
  Serial.flush();
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Trying SSD1306 driver...");
  Serial.flush();

  // Try SSD1306
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 begin() FAILED!");
    Serial.flush();
    while (1) {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }

  Serial.println("SSD1306 initialized OK!");
  Serial.flush();

  // Max brightness
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0xFF);

  // Clear and show test
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEST");
  display.println("SSD1306");
  display.setTextSize(1);
  display.println("If you see this,");
  display.println("display works!");
  display.display();

  Serial.println("Test pattern sent to display!");
  Serial.println("You should see 'TEST SSD1306' on screen");
  Serial.flush();

  delay(3000);
}

void loop() {
  static int count = 0;

  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.print(count);

  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("SSD1306 driver");

  // Animated bar
  display.fillRect(0, 50, (count * 4) % 128, 10, SSD1306_WHITE);

  display.display();

  Serial.print("Frame: ");
  Serial.println(count);

  count++;
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  delay(200);
}
