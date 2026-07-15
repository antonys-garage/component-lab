// MAX7219 Dot Matrix Module ANIMATION PACK

// | MAX7219 | ESP32-C3 |
// | ------- | -------- |
// | VCC     | 5V       |
// | GND     | GND      |
// | DIN     | GPIO 7   |
// | CS      | GPIO 10  |
// | CLK     | GPIO 6   |

#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 1

#define DATA_PIN 7
#define CLK_PIN  6
#define CS_PIN   10

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 8);
  mx.clear();
}

// 🟢 1. Expanding square
void expandSquare() {
  for (int size = 0; size < 4; size++) {
    mx.clear();

    for (int i = size; i < 8 - size; i++) {
      mx.setPoint(size, i, true);
      mx.setPoint(7 - size, i, true);
      mx.setPoint(i, size, true);
      mx.setPoint(i, 7 - size, true);
    }

    delay(150);
  }
}

// 🔄 2. Rotating pattern
void rotatePattern() {
  byte frame[8] = {
    0b10000001,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001
  };

  for (int shift = 0; shift < 8; shift++) {
    mx.clear();
    for (int row = 0; row < 8; row++) {
      mx.setRow(row, frame[(row + shift) % 8]);
    }
    delay(120);
  }
}

// 💧 3. Wave animation
void waveEffect() {
  static int t = 0;

  mx.clear();

  for (int x = 0; x < 8; x++) {
    int y = (sin((x + t) * 0.7) * 3) + 3;
    mx.setPoint(y, x, true);
  }

  t++;
  delay(80);
}

void loop() {
  for (int i = 0; i < 5; i++) expandSquare();
  for (int i = 0; i < 10; i++) rotatePattern();
  for (int i = 0; i < 40; i++) waveEffect();
}