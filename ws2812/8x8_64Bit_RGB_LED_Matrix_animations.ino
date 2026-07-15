// 8x8 RGB LED Matrix | 64Bit WS2812 5050 RGB LED Matrix
// 3 BEST CIRCULAR EFFECTS

// | LED Matrix | ESP32-C3               |
// | ---------- | ---------------------- |
// | VCC        | 5V                     |
// | GND        | GND                    |
// | DIN        | Any GPIO (e.g., GPIO2) |

// Important:
// Use 5V power supply (NOT from ESP directly if many LEDs)
// 330Ω resistor → between GPIO and DIN
// 1000µF capacitor → across VCC & GND

#include <FastLED.h>

#define LED_PIN     2
#define NUM_LEDS    64
#define WIDTH       8
#define HEIGHT      8
#define BRIGHTNESS  120

CRGB leds[NUM_LEDS];

uint16_t XY(uint8_t x, uint8_t y) {
  if (y % 2 == 0) return y * WIDTH + x;
  else return y * WIDTH + (WIDTH - 1 - x);
}

float cx = 3.5;
float cy = 3.5;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void circleWave() {
  static uint8_t t = 0;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {

      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrt(dx*dx + dy*dy);

      uint8_t brightness = sin8(dist * 40 - t);
      leds[XY(x,y)] = CHSV(dist * 40 + t, 255, brightness);
    }
  }

  FastLED.show();
  t += 4;
  delay(40);
}

void rotatingRing() {
  static float angle = 0;

  fadeToBlackBy(leds, NUM_LEDS, 30);

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {

      float dx = x - cx;
      float dy = y - cy;

      float dist = sqrt(dx*dx + dy*dy);

      if (dist > 2.5 && dist < 3.8) {
        float a = atan2(dy, dx);

        if (sin(a * 6 + angle) > 0.7) {
          leds[XY(x,y)] = CRGB::Orange;
        }
      }
    }
  }

  FastLED.show();

  angle += 0.2;
  delay(40);
}

void centerPulse() {
  static uint8_t t = 0;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {

      float dx = x - cx;
      float dy = y - cy;
      float dist = sqrt(dx*dx + dy*dy);

      uint8_t b = sin8(dist * 50 + t);
      leds[XY(x,y)] = CHSV(160, 255, b);
    }
  }

  FastLED.show();
  t += 5;
  delay(40);
}

void loop() {
  for (int i = 0; i < 80; i++) circleWave();
  for (int i = 0; i < 80; i++) rotatingRing();
  for (int i = 0; i < 80; i++) centerPulse();
}
