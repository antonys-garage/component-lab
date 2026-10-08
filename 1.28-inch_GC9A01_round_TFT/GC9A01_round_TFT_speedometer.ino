// | TFT GC9A01 | ESP32-S3 |
// | ---------- | -------: |
// | **VCC**    |     3.3V |
// | **GND**    |      GND |
// | **SCL**    |  GPIO 12 |
// | **SDA**    |  GPIO 11 |
// | **DC**     |  GPIO 10 |
// | **CS**     |   GPIO 9 |
// | **RST**    |   GPIO 8 |

// speedometer animation

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>
#include <math.h>

#define TFT_CS    9
#define TFT_DC   10
#define TFT_RST   8
#define TFT_SCLK 12
#define TFT_MOSI 11

#define SCREEN_W 240
#define SCREEN_H 240

#define CX 120
#define CY 120

#define MAX_SPEED 180

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// Two 240x240 RGB565 buffers.
// ESP32-S3 N16R8 has enough RAM for these.
GFXcanvas16 background(SCREEN_W, SCREEN_H);
GFXcanvas16 frameBuffer(SCREEN_W, SCREEN_H);

float speedValue = 0;
float targetSpeed = 0;

bool accelerating = true;

unsigned long lastFrame = 0;

// --------------------------------------------------
// Angle to coordinate
// --------------------------------------------------

int16_t posX(float angle, float radius) {
  return CX + cos(radians(angle)) * radius;
}

int16_t posY(float angle, float radius) {
  return CY + sin(radians(angle)) * radius;
}

// --------------------------------------------------
// Draw static speedometer
// --------------------------------------------------

void drawDial(GFXcanvas16 &gfx) {

  gfx.fillScreen(GC9A01A_BLACK);

  // Outer rings
  gfx.drawCircle(CX, CY, 118, GC9A01A_DARKGREY);
  gfx.drawCircle(CX, CY, 116, GC9A01A_DARKGREY);
  gfx.drawCircle(CX, CY, 111, GC9A01A_BLUE);

  // Tick marks
  for (int i = 0; i <= 36; i++) {

    float angle = 135.0 + (i * 270.0 / 36.0);

    int outer = 105;
    int inner = (i % 2 == 0) ? 94 : 99;

    float tickSpeed = i * 5.0;

    uint16_t color;

    if (tickSpeed >= 140)
      color = GC9A01A_RED;
    else
      color = GC9A01A_WHITE;

    gfx.drawLine(
      posX(angle, outer),
      posY(angle, outer),
      posX(angle, inner),
      posY(angle, inner),
      color
    );
  }

  // Speed numbers
  gfx.setTextSize(1);

  for (int s = 0; s <= 180; s += 20) {

    float angle =
      135.0 + (s * 270.0 / 180.0);

    int r = 81;

    int x = posX(angle, r);
    int y = posY(angle, r);

    String text = String(s);

    int16_t x1, y1;
    uint16_t w, h;

    gfx.getTextBounds(
      text,
      x,
      y,
      &x1,
      &y1,
      &w,
      &h
    );

    if (s >= 140)
      gfx.setTextColor(GC9A01A_RED);
    else
      gfx.setTextColor(GC9A01A_WHITE);

    gfx.setCursor(
      x - w / 2,
      y - h / 2
    );

    gfx.print(text);
  }

  // Units
  gfx.setTextSize(1);
  gfx.setTextColor(GC9A01A_CYAN);

  gfx.setCursor(103, 145);
  gfx.print("km/h");

  // Bottom branding
  gfx.setTextColor(GC9A01A_DARKGREY);

  gfx.setCursor(92, 190);
  gfx.print("ESP32-S3");

  gfx.setTextColor(GC9A01A_BLUE);

  gfx.setCursor(87, 202);
  gfx.print("SPEEDOMETER");
}

// --------------------------------------------------
// Draw changing parts
// --------------------------------------------------

void drawDynamic(GFXcanvas16 &gfx, float speed) {

  // -----------------------------
  // Digital speed
  // -----------------------------

  String speedText = String((int)speed);

  gfx.setTextSize(4);

  int16_t x1, y1;
  uint16_t w, h;

  gfx.getTextBounds(
    speedText,
    0,
    0,
    &x1,
    &y1,
    &w,
    &h
  );

  if (speed >= 140)
    gfx.setTextColor(GC9A01A_RED);
  else if (speed >= 100)
    gfx.setTextColor(GC9A01A_YELLOW);
  else
    gfx.setTextColor(GC9A01A_WHITE);

  gfx.setCursor(
    CX - w / 2,
    CY - 15
  );

  gfx.print(speedText);

  // -----------------------------
  // Needle
  // -----------------------------

  float angle =
    135.0 + (speed * 270.0 / MAX_SPEED);

  int needleLength = 82;

  int nx = posX(angle, needleLength);
  int ny = posY(angle, needleLength);

  uint16_t needleColor;

  if (speed >= 140)
    needleColor = GC9A01A_RED;
  else
    needleColor = GC9A01A_CYAN;

  // Needle
  gfx.drawLine(
    CX,
    CY,
    nx,
    ny,
    needleColor
  );

  gfx.drawLine(
    CX + 1,
    CY,
    nx + 1,
    ny,
    needleColor
  );

  gfx.drawLine(
    CX,
    CY + 1,
    nx,
    ny + 1,
    needleColor
  );

  // Center hub
  gfx.fillCircle(
    CX,
    CY,
    7,
    GC9A01A_RED
  );

  gfx.fillCircle(
    CX,
    CY,
    3,
    GC9A01A_WHITE
  );
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup() {

  Serial.begin(115200);

  SPI.begin(
    TFT_SCLK,
    -1,
    TFT_MOSI,
    TFT_CS
  );

  tft.begin();

  tft.setRotation(0);

  // Create static dial
  drawDial(background);

  // First frame
  memcpy(
    frameBuffer.getBuffer(),
    background.getBuffer(),
    SCREEN_W * SCREEN_H * 2
  );

  drawDynamic(frameBuffer, 0);

  tft.drawRGBBitmap(
    0,
    0,
    frameBuffer.getBuffer(),
    SCREEN_W,
    SCREEN_H
  );
}

// --------------------------------------------------
// Animation
// --------------------------------------------------

void loop() {

  // Accelerate
  if (accelerating) {

    targetSpeed += 0.6;

    if (targetSpeed >= 180) {
      targetSpeed = 180;
      accelerating = false;
    }

  }

  // Decelerate
  else {

    targetSpeed -= 0.6;

    if (targetSpeed <= 0) {
      targetSpeed = 0;
      accelerating = true;
    }
  }

  // Smooth needle
  speedValue +=
    (targetSpeed - speedValue) * 0.12;

  // ~40 FPS
  if (millis() - lastFrame >= 25) {

    lastFrame = millis();

    // Restore static background
    memcpy(
      frameBuffer.getBuffer(),
      background.getBuffer(),
      SCREEN_W * SCREEN_H * 2
    );

    // Add current needle + speed
    drawDynamic(
      frameBuffer,
      speedValue
    );

    // Send complete frame
    tft.drawRGBBitmap(
      0,
      0,
      frameBuffer.getBuffer(),
      SCREEN_W,
      SCREEN_H
    );
  }
}