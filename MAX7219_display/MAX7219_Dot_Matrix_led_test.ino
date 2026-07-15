// MAX7219 Dot Matrix Module All LEDs ON

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
  mx.control(MD_MAX72XX::INTENSITY, 15); // max brightness
  mx.clear();

  // Turn ON all LEDs
  for (uint8_t row = 0; row < 8; row++) {
    for (uint8_t col = 0; col < 8; col++) {
      mx.setPoint(row, col, true);
    }
  }
}

void loop() {}