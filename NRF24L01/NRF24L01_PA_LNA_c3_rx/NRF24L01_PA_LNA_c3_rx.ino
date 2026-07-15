#include <SPI.h>
#include <RF24.h>

// =========================
// NRF24L01+ PA+LNA
// ESP32-C3 Super Mini
// =========================

// NRF24 -> ESP32-C3
// GND  -> GND
// VCC  -> 3.3V (external regulator recommended)
// CE   -> GPIO4
// CSN  -> GPIO5
// SCK  -> GPIO6
// MOSI -> GPIO7
// MISO -> GPIO2
// IRQ  -> NC

#define CE_PIN    4
#define CSN_PIN   5

#define SCK_PIN   6
#define MISO_PIN  2
#define MOSI_PIN  7

RF24 radio(CE_PIN, CSN_PIN);

const byte address[6] = "00001";

uint32_t packets = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("ESP32-C3 NRF24 Receiver");
  Serial.println("=================================");

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

  if (!radio.begin(&SPI))
  {
    Serial.println("NRF24 NOT FOUND!");
    while (1);
  }

  if (radio.isChipConnected())
    Serial.println("NRF24 Connected");
  else
    Serial.println("Chip Connection Failed");

  radio.setPALevel(RF24_PA_MAX);      // PA+LNA Module
  radio.setDataRate(RF24_250KBPS);    // Longest Range
  radio.setChannel(100);              // Same on TX
  radio.setCRCLength(RF24_CRC_16);
  radio.setRetries(5, 15);

  radio.openReadingPipe(1, address);

  radio.startListening();

  Serial.println("Receiver Ready");
  Serial.println();
}

void loop()
{
  if (radio.available())
  {
    char text[32] = {0};

    radio.read(&text, sizeof(text));

    packets++;

    Serial.print("Packet #");
    Serial.print(packets);

    Serial.print("  Data: ");
    Serial.println(text);
  }
}