// | ESP32-C3 Super Mini | Logic Level Converter | HC-SR04 |
// | ------------------- | --------------------- | ------- |
// | 5V (VBUS)           | HV                    | VCC     |
// | 3.3V                | LV                    | —       |
// | GND                 | GND                   | GND     |
// | GPIO2 (TRIG)        | LV1 → HV1             | TRIG    |
// | GPIO3 (ECHO)        | LV2 ← HV2             | ECHO    |

// ESP32-C3           Logic Level Converter       HC-SR04

// 5V  -------------> HV
// 3.3V ------------> LV
// GND -------------> GND -------------------------> GND

// GPIO2 -----------> LV1 ---- HV1 ---------------> TRIG

// GPIO3 <----------- LV2 ---- HV2 <--------------- ECHO

#define TRIG_PIN 2
#define ECHO_PIN 3

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    Serial.println("No Echo!");
  } else {
    float distance = duration * 0.0343 / 2;
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  delay(500);
}