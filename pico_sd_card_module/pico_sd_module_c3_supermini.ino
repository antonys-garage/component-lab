// SD module pin	ESP32-C3 SuperMini
// 3V3	          3V3
// GND	          GND
// SCK	          GPIO4
// TX(MISO)       GPIO6
// RX(MOSI)       GPIO5
// CS	            GPIO7
// CD	            unconnected

#include <SPI.h>
#include <SD.h>

#define PIN_SCK  4
#define PIN_MISO 6   // module TX
#define PIN_MOSI 5   // module RX
#define PIN_CS   7

void setup() {
  Serial.begin(115200);
  delay(2000);

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  if (!SD.begin(PIN_CS, SPI, 1000000)) {   // start slow at 1 MHz
    Serial.println("SD init failed");
    return;
  }

  Serial.println("SD card OK");
  Serial.printf("Card size: %llu MB\n", SD.cardSize() / (1024 * 1024));

  File f = SD.open("/test.txt", FILE_WRITE);
  if (f) {
    f.println("Hello from ESP32-C3");
    f.close();
  }

  f = SD.open("/test.txt");
  if (f) {
    while (f.available()) Serial.write(f.read());
    f.close();
  }
}

void loop() {}