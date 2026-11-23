#include <Wire.h>

#define BUS        Wire    // Wire = header pins, Wire1 = Qwiic port
#define STTS_ADDR  0x3C    // set from the scanner
#define REG_WHOAMI 0x01
#define REG_CTRL   0x04
#define REG_TEMP_L 0x06

bool writeReg(uint8_t reg, uint8_t val) {
  BUS.beginTransmission(STTS_ADDR);
  BUS.write(reg);
  BUS.write(val);
  return BUS.endTransmission() == 0;
}

bool readRegs(uint8_t reg, uint8_t *buf, uint8_t n) {
  BUS.beginTransmission(STTS_ADDR);
  BUS.write(reg);
  if (BUS.endTransmission(false) != 0) return false;
  if (BUS.requestFrom((uint8_t)STTS_ADDR, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = BUS.read();
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  BUS.begin();

  uint8_t id;
  if (!readRegs(REG_WHOAMI, &id, 1)) {
    Serial.println("No ACK from sensor: wrong address or wiring");
    while (true) {}
  }
  Serial.print("WHO_AM_I: 0x");
  Serial.println(id, HEX);   // expect 0xA0
  writeReg(REG_CTRL, 0x0C);
  delay(100);
}

void loop() {
  uint8_t b[2];
  if (readRegs(REG_TEMP_L, b, 2)) {
    int16_t raw = (int16_t)((b[1] << 8) | b[0]);
    Serial.print("Temp: ");
    Serial.print(raw / 100.0, 2);
    Serial.println(" C");
  } else {
    Serial.println("Read failed");
  }
  delay(1000);
}