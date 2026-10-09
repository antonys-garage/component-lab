#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <math.h>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789  _panel_instance;
  lgfx::Bus_SPI       _bus_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host   = SPI2_HOST;
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;   // 40 MHz SPI
      cfg.pin_sclk   = 18;
      cfg.pin_mosi   = 17;
      cfg.pin_miso   = -1;
      cfg.pin_dc     = 15;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 10;
      cfg.pin_rst          = 16;
      cfg.pin_busy         = -1;
      cfg.panel_width      = 76;
      cfg.panel_height     = 284;
      cfg.offset_x         = 82;
      cfg.offset_y         = 18;
      cfg.readable         = false;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX lcd;
LGFX_Sprite canvas(&lcd);

const int SCREEN_W = 284;
const int SCREEN_H = 76;

// Studio Console Color Palette (16-bit RGB565)
#define C_CHASSIS     0x0000  // Jet Black
#define C_SCOPE_BG    0x0100  // Faint CRT tube back-glow
#define C_SCOPE_GRID  0x0280  // Subdued Oscilloscope Graticule
#define C_PHOSPHOR    0x07E0  // Radiant P7 CRT Green
#define C_PHOSPHOR_DIM 0x0340 // Decaying phosphor trace
#define C_NEEDLE      0xFFFF  // Peak White
#define C_PEAK_RED    0xF800  // Clip / Overload Red
#define C_BAR_MID     0xFD20  // Amber Warning
#define C_BAR_LOW     0x067F  // Clean Studio Cyan

// Equalizer State (20 Bands)
const int BANDS = 20;
float bandLevels[BANDS];
float bandPeaks[BANDS];
int peakHold[BANDS];

// Oscilloscope History Buffer
const int SCOPE_W = 120;
int scopeY[SCOPE_W];

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(1); // 284 x 76 Landscape

  canvas.setColorDepth(16);
  canvas.createSprite(SCREEN_W, SCREEN_H);

  for (int i = 0; i < BANDS; i++) {
    bandLevels[i] = 0.0f;
    bandPeaks[i] = 0.0f;
    peakHold[i] = 0;
  }
  for (int i = 0; i < SCOPE_W; i++) {
    scopeY[i] = 42;
  }
}

void drawOscilloscope(float t) {
  const int X0 = 6;
  const int Y0 = 14;
  const int W = SCOPE_W;
  const int H = 56;
  const int MID_Y = Y0 + (H / 2);

  // CRT Tube Bezel & Graticule
  canvas.fillRect(X0, Y0, W, H, C_SCOPE_BG);
  canvas.drawRect(X0, Y0, W, H, C_SCOPE_GRID);

  // Sub-division lines (analog scope reticle)
  for (int x = X0 + 20; x < X0 + W; x += 20) {
    canvas.drawFastVLine(x, Y0, H, C_SCOPE_GRID);
  }
  canvas.drawFastHLine(X0, MID_Y, W, C_SCOPE_GRID);
  canvas.drawFastHLine(X0, MID_Y - 14, W, 0x0180);
  canvas.drawFastHLine(X0, MID_Y + 14, W, 0x0180);

  // Calculate new wave points (superposition of 3 harmonic frequencies)
  for (int x = 0; x < W; x++) {
    float angle1 = (x * 0.12f) + (t * 4.0f);
    float angle2 = (x * 0.04f) - (t * 2.5f);
    float angle3 = (x * 0.25f) + (t * 6.0f);

    float val = (sinf(angle1) * 0.55f) + (cosf(angle2) * 0.35f) + (sinf(angle3) * 0.10f);
    scopeY[x] = MID_Y + (int)(val * 24.0f);
  }

  // Draw analog trace with persistence glow (dim outer line + bright core line)
  for (int x = 0; x < W - 1; x++) {
    int yA = scopeY[x];
    int yB = scopeY[x + 1];

    // Phosphor glow halo
    canvas.drawLine(X0 + x, yA - 1, X0 + x + 1, yB - 1, C_PHOSPHOR_DIM);
    canvas.drawLine(X0 + x, yA + 1, X0 + x + 1, yB + 1, C_PHOSPHOR_DIM);

    // High-intensity electron beam center
    canvas.drawLine(X0 + x, yA, X0 + x + 1, yB, C_PHOSPHOR);
  }

  // Oscilloscope HUD Tag
  canvas.setTextColor(C_PHOSPHOR);
  canvas.setTextSize(1);
  canvas.drawString("CH1: 50mV/DIV", X0 + 4, Y0 + 4);
}

void drawEqualizer(float t) {
  const int X_START = 138;
  const int Y_BASE = 70;
  const int MAX_H = 54;
  const int BAR_W = 5;
  const int GAP = 2;

  // Spectrum Container Outline
  canvas.drawFastVLine(132, 14, 56, 0x2104);

  for (int i = 0; i < BANDS; i++) {
    // Harmonic wave excitation to emulate musical notes
    float harmonic1 = sinf(t * 3.5f + i * 0.45f) * 0.5f + 0.5f;
    float harmonic2 = cosf(t * 1.8f - i * 0.30f) * 0.5f + 0.5f;
    float noise = (rand() % 15) / 100.0f;

    // Bass response heavier on lower indices, tapering to treble
    float eqCurve = 1.0f - (i * 0.025f);
    float targetHeight = ((harmonic1 * 0.6f + harmonic2 * 0.4f) * eqCurve + noise) * MAX_H;

    // Fast-attack, slow-decay ballistics
    if (targetHeight > bandLevels[i]) {
      bandLevels[i] = targetHeight; // Instant attack
    } else {
      bandLevels[i] -= 1.8f;        // Smooth decay
      if (bandLevels[i] < 2) bandLevels[i] = 2;
    }

    // Peak-hold dynamics with gravity
    if (bandLevels[i] >= bandPeaks[i]) {
      bandPeaks[i] = bandLevels[i];
      peakHold[i] = 16; // Hold frames
    } else {
      if (peakHold[i] > 0) {
        peakHold[i]--;
      } else {
        bandPeaks[i] -= 1.0f; // Gravity fall
        if (bandPeaks[i] < 0) bandPeaks[i] = 0;
      }
    }

    int bx = X_START + (i * (BAR_W + GAP));
    int curH = (int)bandLevels[i];
    int peakY = Y_BASE - (int)bandPeaks[i];

    // Segmented LED look for each spectrum band
    for (int y = 0; y < curH; y += 3) {
      uint16_t segColor;
      int relY = y;

      if (relY > 42) {
        segColor = C_PEAK_RED;  // Overload / Clipping
      } else if (relY > 26) {
        segColor = C_BAR_MID;   // Mid energy
      } else {
        segColor = C_BAR_LOW;   // Nominal level
      }

      canvas.fillRect(bx, Y_BASE - y - 2, BAR_W, 2, segColor);
    }

    // Floating Peak-Hold Needle
    if (peakY < Y_BASE) {
      canvas.drawFastHLine(bx, peakY, BAR_W, C_NEEDLE);
    }
  }

  // EQ Header Readout
  canvas.setTextColor(0x7BEF);
  canvas.drawString("RTA // 20-BAND", X_START + 2, 4);
}

void drawTopConsoleBar() {
  canvas.drawFastHLine(0, 11, SCREEN_W, 0x3186);

  canvas.setTextColor(C_BAR_LOW);
  canvas.setTextSize(1);
  canvas.drawString("DSP.AUDIO", 6, 2);

  canvas.setTextColor(0xFFFF);
  canvas.drawString("48.0 kHz", 70, 2);

  canvas.setTextColor(C_PEAK_RED);
  canvas.drawString("CLIP", 132, 2);

  // Stereo Level Readout
  canvas.setTextColor(0x07E0);
  canvas.drawRightString("-3.2 dB", SCREEN_W - 6, 2);
}

void loop() {
  static float t = 0;
  t += 0.035f;

  canvas.fillSprite(C_CHASSIS);

  drawTopConsoleBar();
  drawOscilloscope(t);
  drawEqualizer(t);

  // Outer framing
  canvas.drawRect(0, 0, SCREEN_W, SCREEN_H, 0x2104);

  // Blit frame directly to panel via SPI DMA
  canvas.pushSprite(0, 0);

  delay(15); // Solid ~60 FPS
}