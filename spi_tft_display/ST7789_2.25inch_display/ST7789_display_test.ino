/*
  Minimal display test — ST7789 76x284 strip on ESP32-S3-N16R8.
  No SD card, no sprites — just confirms the panel + offset patch work.

  If this crashes too, the problem is in the display init/wiring/patch,
  not the SD/video code. If this runs fine, the crash is downstream
  (SD card, SPI contention, or the frame buffer allocation).
*/

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Display test starting...");

  tft.init();
  tft.setRotation(1);          // 284x76 landscape — needs the colstart=18/rowstart=82 patch
  tft.fillScreen(TFT_BLACK);

  Serial.println("init + fillScreen done");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("HELLO", 10, 20);

  tft.fillRect(0, 0, 20, 20, TFT_RED);
  tft.fillRect(20, 0, 20, 20, TFT_GREEN);
  tft.fillRect(40, 0, 20, 20, TFT_BLUE);

  tft.drawRect(0, 0, 284, 76, TFT_YELLOW);   // full-panel border to confirm size/offset

  Serial.println("Test pattern drawn. If colors/position look wrong, it's the colstart/rowstart offset.");
}

void loop() {
  // Nothing here on purpose — static pattern is easiest to inspect
}
