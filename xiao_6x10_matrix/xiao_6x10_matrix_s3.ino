#include <Adafruit_NeoPixel.h>

// On XIAO ESP32-C6, D0 / A0 maps to physical GPIO 0
#define PIN        D0    
#define MATRIX_W   6     // Columns (0..5)
#define MATRIX_H   10    // Rows (0..9)
#define NUMPIXELS  (MATRIX_W * MATRIX_H)

Adafruit_NeoPixel matrix(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const bool SERPENTINE = true;

// Physical hardware coordinate mapping (0..59)
uint16_t getIndex(uint8_t x, uint8_t y) {
  if (x >= MATRIX_W || y >= MATRIX_H) return 0;
  if (SERPENTINE && (y % 2 == 1)) {
    return (y * MATRIX_W) + (MATRIX_W - 1 - x);
  } else {
    return (y * MATRIX_W) + x;
  }
}

void setPixel(uint8_t x, uint8_t y, uint32_t color) {
  matrix.setPixelColor(getIndex(x, y), color);
}

// 24-bit hue wheel helper
uint32_t wheel(byte pos) {
  if (pos < 85) {
    return matrix.Color(pos * 3, 255 - pos * 3, 0);
  } else if (pos < 170) {
    pos -= 85;
    return matrix.Color(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return matrix.Color(0, pos * 3, 255 - pos * 3);
  }
}

void fadeAll(uint8_t scale) {
  for (int i = 0; i < NUMPIXELS; i++) {
    uint32_t c = matrix.getPixelColor(i);
    uint8_t r = (uint8_t)(c >> 16);
    uint8_t g = (uint8_t)(c >> 8);
    uint8_t b = (uint8_t)c;
    matrix.setPixelColor(i, matrix.Color((r * scale) >> 8, (g * scale) >> 8, (b * scale) >> 8));
  }
}

// Helper: Heart renderer defined before effectBeatingHeart
void drawHeart(const uint8_t mask[MATRIX_H][MATRIX_W], float scale) {
  matrix.clear();
  for (uint8_t y = 0; y < MATRIX_H; y++) {
    for (uint8_t x = 0; x < MATRIX_W; x++) {
      uint8_t val = mask[y][x];
      if (val == 2) {
        // Deep hot crimson center
        setPixel(x, y, matrix.Color((uint8_t)(255 * scale), 0, (uint8_t)(40 * scale)));
      } else if (val == 1) {
        // Rose-pink border
        setPixel(x, y, matrix.Color((uint8_t)(200 * scale), (uint8_t)(20 * scale), (uint8_t)(70 * scale)));
      }
    }
  }
  matrix.show();
}

void setup() {
  matrix.begin();
  matrix.setBrightness(30); // 10-15% brightness for stable USB power
  matrix.clear();
  matrix.show();
}

// ================= EFFECT 1: SPIRAL PINWHEEL (CHAKKRAM) =================
void effectSpiralVortex(uint32_t durationMs) {
  uint32_t startTime = millis();
  const float cx = 2.5f;
  const float cy = 4.5f;
  float rotationAngle = 0.0f;
  const uint8_t arms = 3;

  while (millis() - startTime < durationMs) {
    matrix.clear();

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        float dx = x - cx;
        float dy = (y - cy) * 0.7f;
        float r = sqrt(dx * dx + dy * dy);
        float angle = atan2(dy, dx);
        if (angle < 0) angle += 2 * PI;

        float spiralVal = sin((angle * arms) - (r * 1.5f) + rotationAngle);
        if (spiralVal > 0.4f) {
          uint8_t intensity = (uint8_t)(map((int)(spiralVal * 100), 40, 100, 40, 255));
          byte hue = (byte)((int)(r * 35 + rotationAngle * 20) & 255);
          uint32_t c = wheel(hue);
          uint8_t red = ((uint8_t)(c >> 16) * intensity) >> 8;
          uint8_t green = ((uint8_t)(c >> 8) * intensity) >> 8;
          uint8_t blue = ((uint8_t)c * intensity) >> 8;
          setPixel(x, y, matrix.Color(red, green, blue));
        }
      }
    }

    matrix.show();
    rotationAngle += 0.25f;
    delay(30);
  }
}

// ================= EFFECT 2: DOUBLE HELIX SINE WAVE =================
void effectDoubleHelix(uint32_t durationMs) {
  uint32_t startTime = millis();
  float phase = 0.0f;

  while (millis() - startTime < durationMs) {
    fadeAll(160);

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      float wave1 = sin(phase + (y * 0.7f));
      int x1 = (int)round(2.5f + (wave1 * 2.2f));

      float wave2 = sin(phase + (y * 0.7f) + PI);
      int x2 = (int)round(2.5f + (wave2 * 2.2f));

      if (x1 >= 0 && x1 < MATRIX_W) setPixel(x1, y, matrix.Color(0, 240, 180));
      if (x2 >= 0 && x2 < MATRIX_W) setPixel(x2, y, matrix.Color(255, 30, 90));

      if (abs(x1 - x2) <= 1) {
        setPixel(constrain((x1 + x2) / 2, 0, MATRIX_W - 1), y, matrix.Color(255, 220, 0));
      }
    }

    matrix.show();
    phase += 0.22f;
    delay(40);
  }
}

// ================= EFFECT 3: DIAMOND EXPANSION =================
void effectExpandingDiamond(uint32_t durationMs) {
  uint32_t startTime = millis();
  const float cx = 2.5f;
  const float cy = 4.5f;
  float dRadius = 0.0f;
  byte colorIdx = 0;

  uint32_t diamondColors[] = {
    matrix.Color(255, 100, 0),
    matrix.Color(0, 200, 255),
    matrix.Color(180, 0, 255),
    matrix.Color(0, 255, 80)
  };

  while (millis() - startTime < durationMs) {
    matrix.clear();

    for (uint8_t y = 0; y < MATRIX_H; y++) {
      for (uint8_t x = 0; x < MATRIX_W; x++) {
        float manhattan = abs(x - cx) + (abs(y - cy) * 0.65f);
        float diff = abs(manhattan - dRadius);

        if (diff < 0.7f) {
          uint8_t brightness = (uint8_t)((1.0f - (diff / 0.7f)) * 255);
          uint32_t col = diamondColors[colorIdx];
          uint8_t r = ((uint8_t)(col >> 16) * brightness) >> 8;
          uint8_t g = ((uint8_t)(col >> 8) * brightness) >> 8;
          uint8_t b = ((uint8_t)col * brightness) >> 8;
          setPixel(x, y, matrix.Color(r, g, b));
        }
      }
    }

    matrix.show();
    dRadius += 0.25f;
    if (dRadius > 6.0f) {
      dRadius = 0.0f;
      colorIdx = (colorIdx + 1) % 4;
    }
    delay(35);
  }
}

// ================= EFFECT 4: GRAPHIC VU EQUALIZER =================
void effectEqualizerPeaks(uint32_t durationMs) {
  uint32_t startTime = millis();
  float levels[MATRIX_W] = {2, 4, 6, 5, 3, 1};
  float peaks[MATRIX_W]  = {2, 4, 6, 5, 3, 1};
  float targets[MATRIX_W];

  for (int i = 0; i < MATRIX_W; i++) targets[i] = random(1, MATRIX_H);

  while (millis() - startTime < durationMs) {
    matrix.clear();

    for (int x = 0; x < MATRIX_W; x++) {
      levels[x] += (targets[x] - levels[x]) * 0.35f;
      if (abs(levels[x] - targets[x]) < 0.5f) {
        targets[x] = random(1, MATRIX_H);
      }

      if (levels[x] > peaks[x]) {
        peaks[x] = levels[x];
      } else {
        peaks[x] -= 0.12f;
        if (peaks[x] < 0) peaks[x] = 0;
      }

      int curH = (int)round(levels[x]);
      int peakH = (int)round(peaks[x]);

      for (int y = MATRIX_H - 1; y >= MATRIX_H - curH; y--) {
        if (y > 6) {
          setPixel(x, y, matrix.Color(0, 180, 0));
        } else if (y > 2) {
          setPixel(x, y, matrix.Color(220, 180, 0));
        } else {
          setPixel(x, y, matrix.Color(255, 10, 0));
        }
      }

      int peakY = MATRIX_H - 1 - peakH;
      if (peakY >= 0 && peakY < MATRIX_H) {
        setPixel(x, peakY, matrix.Color(255, 255, 255));
      }
    }

    matrix.show();
    delay(40);
  }
}

// ================= EFFECT 5: PULSATING HEART =================
void effectBeatingHeart(uint32_t durationMs) {
  uint32_t startTime = millis();
  
  const uint8_t heartMask[MATRIX_H][MATRIX_W] = {
    {0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 1, 0},
    {1, 2, 1, 1, 2, 1},
    {1, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 1},
    {0, 1, 2, 2, 1, 0},
    {0, 0, 1, 1, 0, 0},
    {0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0}
  };

  while (millis() - startTime < durationMs) {
    // Pulse 1
    for (float b = 0.3f; b <= 1.0f; b += 0.15f) {
      drawHeart(heartMask, b);
      delay(20);
    }
    for (float b = 1.0f; b >= 0.4f; b -= 0.15f) {
      drawHeart(heartMask, b);
      delay(20);
    }

    // Pulse 2
    for (float b = 0.4f; b <= 0.85f; b += 0.15f) {
      drawHeart(heartMask, b);
      delay(20);
    }
    for (float b = 0.85f; b >= 0.15f; b -= 0.1f) {
      drawHeart(heartMask, b);
      delay(25);
    }

    drawHeart(heartMask, 0.15f);
    delay(380);
  }
}

// ================= MAIN LOOP =================
void loop() {
  effectSpiralVortex(6500);
  effectDoubleHelix(6500);
  effectExpandingDiamond(6500);
  effectEqualizerPeaks(6500);
  effectBeatingHeart(6500);
}