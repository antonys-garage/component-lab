// SD module pin	Pico 2 W pin
// 3V3	          3V3(OUT), pin 36
// GND	          any GND
// SCK	          GP18
// TX (MISO)	    GP16
// RX (MOSI)	    GP19
// CS	          GP17
// CD	          unconnected

#include <SPI.h>
#include <SD.h>

const int PIN_MISO = 16;  // module TX
const int PIN_CS   = 17;
const int PIN_SCK  = 18;
const int PIN_MOSI = 19;  // module RX

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  SPI.setRX(PIN_MISO);
  SPI.setSCK(PIN_SCK);
  SPI.setTX(PIN_MOSI);

  if (!SD.begin(PIN_CS)) {
    Serial.println("SD init failed");
    return;
  }
  Serial.println("SD card OK");

  File f = SD.open("/test.txt", FILE_WRITE);
  if (f) {
    f.println("Hello from Pico 2 W");
    f.close();
  } else {
    Serial.println("Could not open file for writing");
    return;
  }

  f = SD.open("/test.txt");
  if (f) {
    while (f.available()) Serial.write(f.read());
    f.close();
  }
}

void loop() {}