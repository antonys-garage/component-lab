// | NRF24L01 | Arduino Nano |
// | -------- | ------------ |
// | GND      | GND          |
// | VCC      | **3.3V**     |
// | CE       | D9           |
// | CSN (CS) | D10          |
// | SCK      | D13          |
// | MOSI     | D11          |
// | MISO     | D12          |
// | IRQ      | NC           |

#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);

  if (!radio.begin()) {
    Serial.println("NRF24 not found!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.stopListening();
}

void loop() {
  const char text[] = "Hello";
  bool ok = radio.write(&text, sizeof(text));

  if (ok)
    Serial.println("Sent");
  else
    Serial.println("Failed");

  delay(1000);
}