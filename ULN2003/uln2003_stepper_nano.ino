// Ardunio Nano + ULN2003 Stepper Motor Driver + 28BYJ-48 5V stepper motor
// | ULN2003 | Arduino Nano |
// | ------- | ------------ |
// | IN1     | D8           |
// | IN2     | D9           |
// | IN3     | D10          |
// | IN4     | D11          |
// | +       | 5V           |
// | -       | GND          |

// Test Code (Rotate Clockwise)

#include <Stepper.h>

const int stepsPerRevolution = 2048;

Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

void setup() {
  Serial.begin(115200);
  myStepper.setSpeed(10); // RPM
  Serial.println("Stepper Test");
}

void loop() {
  Serial.println("Clockwise");
  myStepper.step(2048);   // One full revolution
  delay(1000);

  Serial.println("Counter Clockwise");
  myStepper.step(-2048);
  delay(1000);
}