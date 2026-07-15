// | MAX7219 | ESP32-C3     |
// | ------- | ------------ |
// | VCC     | 5V           |
// | GND     | GND          |
// | DIN     | GPIO7 (MOSI) |
// | CLK     | GPIO6 (SCK)  |
// | CS      | GPIO5        |

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define DATA_PIN 7
#define CLK_PIN  6
#define CS_PIN   5

MD_Parola display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  display.begin();
  display.setIntensity(3);        // Brightness (0-15)
  display.displayClear();

  display.displayText(
    "ANTONY'S GARAGE",
    PA_CENTER,
    60,       // Speed (lower = faster)
    1000,     // Pause
    PA_SCROLL_LEFT,
    PA_SCROLL_LEFT
  );
}

void loop() {
  if (display.displayAnimate()) {
    display.displayReset();
  }
}