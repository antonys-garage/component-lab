#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define DATA_PIN 7
#define CLK_PIN 6
#define CS_PIN 5

MD_MAX72XX mx(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 5);

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 32; col++) {
      mx.setPoint(row, col, true);
    }
  }
}

void loop() {}