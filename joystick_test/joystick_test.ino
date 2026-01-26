// Joystick Wiring Diagnostic
// Shows raw values for each GPIO to help identify correct wiring

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 6
#define SCL_PIN 7
#define LED_PIN 8

// Current pin assignments
#define JOY1_VRX 0
#define JOY1_VRY 1
#define JOY1_SW 2
#define JOY2_VRX 3
#define JOY2_VRY 4
#define JOY2_SW 5

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n=== Joystick Wiring Diagnostic ===");
  Serial.println("Move ONE joystick axis at a time and watch which value changes");
  Serial.println("");

  pinMode(LED_PIN, OUTPUT);
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SCREEN_ADDRESS, true);
  display.setContrast(255);
  display.clearDisplay();
  display.display();
}

void loop() {
  int gpio0 = analogRead(0);
  int gpio1 = analogRead(1);
  int gpio2_sw = digitalRead(2);
  int gpio3 = analogRead(3);
  int gpio4 = analogRead(4);
  int gpio5_sw = digitalRead(5);

  // Serial output
  Serial.println("----------------------------------------");
  Serial.print("GPIO0 (J1_X?): "); Serial.println(gpio0);
  Serial.print("GPIO1 (J1_Y?): "); Serial.println(gpio1);
  Serial.print("GPIO2 (J1_SW): "); Serial.println(gpio2_sw == LOW ? "PRESSED" : "released");
  Serial.print("GPIO3 (J2_X?): "); Serial.println(gpio3);
  Serial.print("GPIO4 (J2_Y?): "); Serial.println(gpio4);
  Serial.print("GPIO5 (J2_SW): "); Serial.println(gpio5_sw == LOW ? "PRESSED" : "released");
  Serial.println("");

  // Display output
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 0);
  display.println("WIRING DIAGNOSTIC");
  display.println("Move 1 axis at a time!");
  display.println("");

  display.print("GPIO0: "); display.println(gpio0);
  display.print("GPIO1: "); display.println(gpio1);
  display.print("GPIO2: "); display.println(gpio2_sw == LOW ? "BTN" : "---");

  display.print("GPIO3: "); display.println(gpio3);
  display.print("GPIO4: "); display.println(gpio4);
  display.print("GPIO5: "); display.println(gpio5_sw == LOW ? "BTN" : "---");

  display.display();

  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  delay(200);
}
