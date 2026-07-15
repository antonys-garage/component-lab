// | ePaper     | ESP32-S3 |
// | ---------- | -------- |
// | VCC        | 3.3V     |
// | GND        | GND      |
// | SDA (MOSI) | GPIO11   |
// | SCL (SCK)  | GPIO12   |
// | CS         | GPIO10   |
// | DC         | GPIO9    |
// | RST        | GPIO8    |
// | BUSY       | GPIO7    |

#include <WiFi.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <time.h>

//====================
// WIFI
//====================

const char* ssid = "ssid";
const char* password = "pass";


//====================
// EPD PINS
//====================

#define EPD_CS      10
#define EPD_DC       9
#define EPD_RST      8
#define EPD_BUSY     7

#define EPD_MOSI    11
#define EPD_CLK     12

SPIClass hspi(FSPI);

//====================
// DISPLAY
//====================

GxEPD2_BW<
    GxEPD2_154_D67,
    GxEPD2_154_D67::HEIGHT
> display(
    GxEPD2_154_D67(
        EPD_CS,
        EPD_DC,
        EPD_RST,
        EPD_BUSY
    )
);

void connectWiFi()
{
    WiFi.begin(ssid,password);

    Serial.print("Connecting");

    while(WiFi.status()!=WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected");

    Serial.println(WiFi.localIP());
}

void syncClock()
{
    configTime(
        19800,
        0,
        "pool.ntp.org",
        "time.nist.gov"
    );
}

void drawStartup()
{
    display.setFullWindow();

    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);

        display.setTextColor(GxEPD_BLACK);

        display.setTextSize(2);

        display.setCursor(20,50);
        display.println("Micro");

        display.setCursor(20,80);
        display.println("Search");

        display.drawLine(20,90,180,90,GxEPD_BLACK);

        display.setTextSize(1);

        display.setCursor(20,115);
        display.println("ESP32-S3");

        display.setCursor(20,130);
        display.println("Weather Dashboard");

    }
    while(display.nextPage());
}

void setup()
{
    Serial.begin(115200);

    hspi.begin(
        EPD_CLK,
        -1,
        EPD_MOSI,
        EPD_CS
    );

    display.epd2.selectSPI(
        hspi,
        SPISettings(
            4000000,
            MSBFIRST,
            SPI_MODE0
        )
    );

    display.init();

    display.setRotation(2);

    drawStartup();

    connectWiFi();

    syncClock();

    delay(3000);
}

void loop()
{

}