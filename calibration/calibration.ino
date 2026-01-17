// Simple joystick calibration sketch
#define JOY1_VRX 0
#define JOY1_VRY 1
#define JOY2_VRX 3
#define JOY2_VRY 4

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Joystick Calibration ===");
  Serial.println("Don't touch the joysticks!");
  Serial.println("");
}

void loop() {
  int j1x = analogRead(JOY1_VRX);
  int j1y = analogRead(JOY1_VRY);
  int j2x = analogRead(JOY2_VRX);
  int j2y = analogRead(JOY2_VRY);

  Serial.print("J1: X=");
  Serial.print(j1x);
  Serial.print("  Y=");
  Serial.print(j1y);
  Serial.print("  |  J2: X=");
  Serial.print(j2x);
  Serial.print("  Y=");
  Serial.println(j2y);

  delay(500);
}
