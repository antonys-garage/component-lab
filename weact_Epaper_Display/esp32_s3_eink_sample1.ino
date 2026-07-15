#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define EPD_CS     10
#define EPD_DC      9
#define EPD_RST     8
#define EPD_BUSY    7

#define EPD_SCK    12
#define EPD_MOSI   11

SPIClass hspi(FSPI);

GxEPD2_BW<
    GxEPD2_154_D67,
    GxEPD2_154_D67::HEIGHT
> display(GxEPD2_154_D67(
    EPD_CS,
    EPD_DC,
    EPD_RST,
    EPD_BUSY
));

void setup()
{
    Serial.begin(115200);

    hspi.begin(
        EPD_SCK,
        -1,
        EPD_MOSI,
        EPD_CS
    );

    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));

    display.init();

    display.setRotation(3);

    display.setFullWindow();

    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);

        display.setTextColor(GxEPD_BLACK);

        display.setTextSize(2);

        display.setCursor(20,50);
        display.println("Antonys Garage");

        display.setCursor(10,80);
        display.println("The Learning Hub");

        display.setCursor(20,120);
        display.println("WeAct 1.54");

    }
    while(display.nextPage());
}

void loop()
{
}