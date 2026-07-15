#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#define LED 8

const char* ssid = "ssid";
const char* password = "password";

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0);

void setup() {

  Wire.begin(5,6);

  display.begin();

  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tf);
  display.drawStr(0,15,"Hello");
  display.drawStr(0,30,"ESP32-C3");
  display.sendBuffer();
  pinMode(LED, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(WiFi.localIP());
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(500);

  digitalWrite(LED, LOW);
  delay(500);
}
