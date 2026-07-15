// | TFT Pin  | ESP32-S3             |
// | -------- | -------------------- |
// | VCC      | 3.3V                 |
// | GND      | GND                  |
// | SCK      | **GPIO12**           |
// | SDA      | **GPIO11**           |
// | RES      | **GPIO9**            |
// | A0       | **GPIO10**           |
// | CS       | **GPIO8** *(or GND)* |
// | LED      | 3.3V                 |


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define TFT_CS   8     // or -1 if tied to GND
#define TFT_DC   10
#define TFT_RST  9
#define TFT_MOSI 11
#define TFT_SCLK 12

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Init display (most important line)
  tft.initR(INITR_BLACKTAB);

  tft.setRotation(1);

  // Color test
  tft.fillScreen(ST77XX_RED);
  delay(1000);

  tft.fillScreen(ST77XX_GREEN);
  delay(1000);

  tft.fillScreen(ST77XX_BLUE);
  delay(1000);

  // Text test
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.println("WORKING");
}

void loop() {}