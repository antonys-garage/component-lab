// | TFT GC9A01 | ESP32-S3 |
// | ---------- | -------: |
// | **VCC**    |     3.3V |
// | **GND**    |      GND |
// | **SCL**    |  GPIO 12 |
// | **SDA**    |  GPIO 11 |
// | **DC**     |  GPIO 10 |
// | **CS**     |   GPIO 9 |
// | **RST**    |   GPIO 8 |

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>

#define TFT_CS    9
#define TFT_DC   10
#define TFT_RST   8
#define TFT_SCLK 12
#define TFT_MOSI 11

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(0);

  tft.fillScreen(GC9A01A_BLACK);

  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.setCursor(55, 105);
  tft.println("ESP32-S3");

  tft.setCursor(65, 130);
  tft.println("GC9A01");
}

void loop() {
  // Color test
  tft.fillScreen(GC9A01A_RED);
  delay(500);

  tft.fillScreen(GC9A01A_GREEN);
  delay(500);

  tft.fillScreen(GC9A01A_BLUE);
  delay(500);

  tft.fillScreen(GC9A01A_BLACK);
  delay(500);

  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  tft.setCursor(55, 105);
  tft.println("TEST OK");

  delay(1000);
}