# ADIY Micro SD Card Module – Pico 2 W & ESP32-C3 Examples

A compact micro SD card module with a 10-pin breakout, used here with the **Raspberry Pi Pico 2 W** and the **ESP32-C3 SuperMini** through the Arduino IDE. This repo has the wiring, test sketches and troubleshooting notes.

By **Antony's Garage** ([@code_antony](https://github.com/code_antony))

---

## Features

- Micro SD card slot
- SPI interface, 3.3 V operation
- 10-pin breakout (two rows of 5), labelled on the back
- Pins for clock, chip select and two data lines, plus a separate card-detect pin (CD)
- Onboard SMD resistors beside the slot
- Pico-style footprint, works with Pico and ESP32 boards

---

## Pinout (back view)

| Left column | Right column |
|---|---|
| 3V3 | RX |
| (unused) | TX |
| (unused) | CS |
| (unused) | GND |
| CD | SCK |

> **Note:** The RX/TX labels are from the module's point of view.
> **TX = MISO** and **RX = MOSI**.

---

## Wiring

| SD module | ESP32-C3 SuperMini | Raspberry Pi Pico 2 W |
|---|---|---|
| 3V3 | 3V3 | 3V3(OUT), pin 36 |
| GND | GND | any GND |
| SCK | GPIO4 | GP18 |
| TX (MISO) | GPIO6 | GP16 |
| RX (MOSI) | GPIO5 | GP19 |
| CS | GPIO7 | GP17 |
| CD | not connected | not connected |

Use **3.3 V only**. Do not connect the module to 5 V.

---

## Arduino IDE setup

### ESP32-C3 SuperMini
1. Install **esp32 by Espressif Systems** in Boards Manager.
2. Select **ESP32C3 Dev Module**.
3. Set **USB CDC On Boot: Enabled**.

### Raspberry Pi Pico 2 W
1. Add this URL under File > Preferences > Additional Boards Manager URLs:
   `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
2. Install **Raspberry Pi Pico/RP2040/RP2350** by Earle F. Philhower, III.
3. Select **Raspberry Pi Pico 2 W**.

The `SPI` and `SD` libraries are included with both cores, so no extra installs are needed.

---

## Example: ESP32-C3 SuperMini

```cpp
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

  if (!SD.begin(PIN_CS, SPI, 1000000)) {
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
```

---

## Example: Raspberry Pi Pico 2 W

```cpp
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
  }

  f = SD.open("/test.txt");
  if (f) {
    while (f.available()) Serial.write(f.read());
    f.close();
  }
}

void loop() {}
```

Open the Serial Monitor at **115200 baud**. Expected output:

```
SD card OK
Hello from ...
```

---

## SD card preparation

- Format the card as **FAT32**.
- Cards of **32 GB or smaller** work most reliably in SPI mode.
- If a card is exFAT or larger than 32 GB, reformat it with the [SD Memory Card Formatter](https://www.sdcard.org/downloads/formatter/).
- Push the card in until it clicks.

---

## Troubleshooting

| Symptom | What to check |
|---|---|
| `SD init failed` | Swap the MISO and MOSI wires, confirm TX = MISO and RX = MOSI |
| Fails on every attempt | Reformat the card as FAT32, try a smaller card, reseat the card |
| Intermittent failures | Shorten jumper wires, check solder joints on the header pins |
| Nothing in Serial Monitor (C3) | Enable **USB CDC On Boot** |
| No Serial output (Pico) | Wait for the port to enumerate, or increase the startup delay |

If init still fails, try a lower SPI speed (for example 400 kHz) on `SD.begin()`.

---

## Use cases

- Compact data loggers
- Audio and image storage
- Wi-Fi file servers (Pico 2 W / ESP32-C3)
- Configuration and log files for embedded projects

---

## License

Add your preferred license here (for example MIT).
