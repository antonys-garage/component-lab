# STTS22H Temperature Sensor with Arduino Uno R4 WiFi

Read temperature from an ST **STTS22H** digital temperature sensor over I2C on an Arduino Uno R4 WiFi, using plain register access (no library needed). Output is printed to the Serial Monitor in °C.

## Hardware

| Part | Notes |
| --- | --- |
| Arduino Uno R4 WiFi | Also works on other boards with a few pin changes |
| SmartElex STTS22H breakout (Robocraze, SKU TIFSS0384) | I2C, 1.5 to 3.6 V supply |
| Qwiic cable with bare ends | Or any four jumper wires |

> **The sensor is a 3.3 V part. Never power it from 5 V.**

## Wiring

Qwiic cable (bare end) to the Uno R4 header:

| Sensor / Qwiic wire | Uno R4 pin |
| --- | --- |
| GND (black) | GND |
| VCC (red) | **3.3V** |
| SDA (blue) | A4 (or the SDA pin beside AREF) |
| SCL (yellow) | A5 (or the SCL pin beside AREF) |

The header I2C pins use the `Wire` bus. The Uno R4 WiFi's Qwiic port is a separate bus, `Wire1`. If you plug the sensor into the Qwiic port instead, change `#define BUS Wire` to `#define BUS Wire1`.

## I2C address

The address on this module was **0x3C**, found with an I2C scanner. The STTS22H has more than one selectable address, so if yours differs, run a scanner and update `STTS_ADDR`.

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Wire.begin();
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(a, HEX);
    }
  }
}

void loop() {}
```

## Code

Sketch: `stts22h_uno_r4.ino`

Key settings at the top of the file:

```cpp
#define BUS        Wire    // Wire = header pins, Wire1 = Qwiic port
#define STTS_ADDR  0x3C    // set from the scanner
#define REG_WHOAMI 0x01
#define REG_CTRL   0x04
#define REG_TEMP_L 0x06
```

How it works:

1. `setup()` reads the WHO_AM_I register. If the sensor does not acknowledge, the sketch prints an error and stops instead of showing a fake value.
2. It writes the control register to enable free-run mode with address auto-increment.
3. `loop()` reads two bytes from the temperature registers once per second, combines them into a signed 16-bit value, and divides by 100 (0.01 °C per LSB).

## Usage

1. Wire the sensor as shown above.
2. Open `stts22h_uno_r4.ino` in Arduino IDE and select **Arduino UNO R4 WiFi**.
3. Set `STTS_ADDR` to your sensor's address.
4. Upload and open the Serial Monitor at **115200 baud**.

Expected output:

```
WHO_AM_I: 0xA0
Temp: 27.31 C
Temp: 27.33 C
...
```

Warm the sensor with a fingertip and the reading should climb.

## Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| `No ACK from sensor` | Wrong address, loose wire, or SDA/SCL swapped |
| `Temp: -0.01 C` repeating | No device answering (raw read of 0xFFFF). Check the bus and address |
| Scanner finds nothing | Check 3.3 V power and cable seating, and make sure you scan the right bus |
| Scanner finds the device on `Wire1` but not `Wire` | Sensor is on the Qwiic port, so set `BUS` to `Wire1` |
| Unexpected WHO_AM_I value | Verify register addresses against the datasheet |

## Notes

- Register addresses and the control value in the sketch are from the STTS22H register map. Check them against the datasheet if you change modes (for example, one-shot or low-power operation).
- Datasheet: linked from the product page on Robocraze.

## License

MIT
