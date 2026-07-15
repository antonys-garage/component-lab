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

#include <WiFi.h>
#include <HTTPClient.h>
#include <JPEGDecoder.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// WiFi
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";

// ✅ Working image source
const char* imageUrl = "http://httpbin.org/image/jpeg";

// TFT
#define TFT_CS   8
#define TFT_DC   10
#define TFT_RST  9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  SPI.begin(12, 13, 11);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  showImage();
}

void loop() {}

void showImage() {

  WiFiClient client;
  HTTPClient http;

  http.begin(client, imageUrl);

  int httpCode = http.GET();

  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode != 200) {
    Serial.println("HTTP ERROR");
    return;
  }

  int len = http.getSize();

  Serial.print("Image size: ");
  Serial.println(len);

  uint8_t *buffer = (uint8_t*) malloc(len);
  if (!buffer) {
    Serial.println("Memory fail");
    return;
  }

  WiFiClient *stream = http.getStreamPtr();
  stream->readBytes(buffer, len);

  if (JpegDec.decodeArray(buffer, len) == 0) {
    Serial.println("Decode failed");
    free(buffer);
    return;
  }

  Serial.println("Decode OK");

  tft.fillScreen(ST77XX_BLACK);

  while (JpegDec.read()) {

    uint16_t *pImg = JpegDec.pImage;

    int x = JpegDec.MCUx * JpegDec.MCUWidth;
    int y = JpegDec.MCUy * JpegDec.MCUHeight;

    int w = JpegDec.MCUWidth;
    int h = JpegDec.MCUHeight;

    if (x < tft.width() && y < tft.height()) {
      tft.drawRGBBitmap(x, y, pImg, w, h);
    }
  }

  free(buffer);
  http.end();

  Serial.println("Image displayed!");
}