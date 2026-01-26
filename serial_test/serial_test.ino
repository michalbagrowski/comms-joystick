// Minimal Serial Test for ESP32-C3
// Tests if Serial output works

#define LED_PIN 8

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);

  // Blink while waiting for serial
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }

  Serial.println("=== Serial Test Starting ===");
  Serial.println("If you see this, Serial works!");
  Serial.flush();
}

void loop() {
  static unsigned long lastPrint = 0;
  static int count = 0;

  if (millis() - lastPrint > 1000) {
    Serial.print("Count: ");
    Serial.println(count++);
    Serial.flush();

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastPrint = millis();
  }
}
