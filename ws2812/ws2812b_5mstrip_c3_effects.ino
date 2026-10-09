#include <FastLED.h>

#define LED_PIN     4
#define NUM_LEDS    300
#define BRIGHTNESS  100
#define EFFECT_TIME 8000   // ms per effect before switching

CRGB leds[NUM_LEDS];
uint8_t effectIndex = 0;
unsigned long lastSwitch = 0;
uint8_t hue = 0;

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
}

void loop() {
  if (millis() - lastSwitch > EFFECT_TIME) {
    effectIndex = (effectIndex + 1) % 6;
    lastSwitch = millis();
    FastLED.clear();
  }

  switch (effectIndex) {
    case 0: rainbowCycle();     break;
    case 1: theaterChase();     break;
    case 2: colorWipe();        break;
    case 3: twinkle();          break;
    case 4: fire();             break;
    case 5: bouncingDot();      break;
  }

  FastLED.show();
  delay(15);
}

// 1. Smooth rainbow flowing down the strip
void rainbowCycle() {
  hue++;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 256 / NUM_LEDS), 255, 255);
  }
}

// 2. Classic theater-marquee chase
void theaterChase() {
  static uint8_t offset = 0;
  offset = (offset + 1) % 3;
  hue++;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ((i + offset) % 3 == 0) ? CRGB(CHSV(hue, 255, 255)) : CRGB::Black;
  }
}

// 3. Color wipe filling the strip end to end, then reversing
void colorWipe() {
  static int pos = 0;
  static int dir = 1;
  static uint8_t wipeHue = 0;

  leds[pos] = CHSV(wipeHue, 255, 255);
  pos += dir;
  if (pos >= NUM_LEDS - 1 || pos <= 0) {
    dir = -dir;
    wipeHue += 40;
  }
}

// 4. Random twinkling stars
void twinkle() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  if (random8() < 60) {
    leds[random16(NUM_LEDS)] = CHSV(random8(), 200, 255);
  }
}

// 5. Fire effect (heat simulation, warm colors)
void fire() {
  static byte heat[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i], random8(0, 10));
  }
  for (int i = NUM_LEDS - 1; i >= 2; i--) {
    heat[i] = (heat[i - 1] + heat[i - 2] + heat[i - 2]) / 3;
  }
  if (random8() < 120) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = HeatColor(heat[i]);
  }
}

// 6. Single bright dot bouncing back and forth with a fading tail
void bouncingDot() {
  static int pos = 0;
  static int dir = 1;

  fadeToBlackBy(leds, NUM_LEDS, 40);
  leds[pos] = CRGB::White;
  pos += dir;
  if (pos >= NUM_LEDS - 1 || pos <= 0) dir = -dir;
}
