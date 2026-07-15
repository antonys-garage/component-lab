// | NRF24L01 | ESP32-C3 |
// | -------- | -------- |
// | GND      | GND      |
// | VCC      | 3.3V     |
// | CE       | GPIO4    |
// | CSN      | GPIO5    |
// | SCK      | GPIO6    |
// | MOSI     | GPIO7    |
// | MISO     | GPIO2    |
// | IRQ      | NC       |


#include <SPI.h>
#include <RF24.h>

RF24 radio(4, 5); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(115200);

  SPI.begin(6, 2, 7, 5); // SCK, MISO, MOSI, SS

  if (!radio.begin()) {
    Serial.println("NRF24 not found!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(0, address);
  radio.startListening();

  Serial.println("Receiver Ready");
}

void loop() {
  if (radio.available()) {
    char text[32] = "";

    radio.read(&text, sizeof(text));

    Serial.print("Received: ");
    Serial.println(text);
  }
}