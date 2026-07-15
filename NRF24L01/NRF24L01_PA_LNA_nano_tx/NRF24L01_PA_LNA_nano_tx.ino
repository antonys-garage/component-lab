#include <SPI.h>
#include <RF24.h>

#define CE_PIN   9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

const byte address[6] = "00001";

uint32_t packetCount = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("Arduino Nano NRF24 Transmitter");
  Serial.println("=================================");

  if (!radio.begin())
  {
    Serial.println("NRF24 NOT FOUND!");
    while (1);
  }

  Serial.println("NRF24 Connected");

  // Recommended settings for PA+LNA communication
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(100);
  radio.setCRCLength(RF24_CRC_16);
  radio.setRetries(5, 15);

  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Transmitter Ready");
}

void loop()
{
  char text[32];

  sprintf(text, "Hello %lu", packetCount);

  bool ok = radio.write(&text, sizeof(text));

  if (ok)
  {
    Serial.print("Sent: ");
    Serial.println(text);
  }
  else
  {
    Serial.println("Send Failed!");
  }

  packetCount++;

  delay(1000);
}