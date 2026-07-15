// | TFT Pin  | ESP32-S3             |
// | -------- | -------------------- |
// | VCC      | 3.3V                 |
// | GND      | GND                  |
// | SCK      | **GPIO12**           |
// | SDA      | **GPIO11**           |
// | RES      | **GPIO9**            |
// | A0       | **GPIO10**           |
// | CS       | **GPIO8** *(or GND)* |
// | LED      | 3.3V                 |

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// TFT Pins
#define TFT_CS   8
#define TFT_DC   10
#define TFT_RST  9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Dot spacing and face parameters
#define DOT 6
int cx = 80;
int cy = 64;
int radius = 40;

void setup() {
  SPI.begin(12, 13, 11);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // Draw the initial static face
  drawDottedFaceBase();
  animateEyes(0);  // 0 = Center
  animateMouth(0); // 0 = Smile
}

void loop() {
  delay(2000);      
  
  // 1. Quick blink
  animateEyes(1);   // 1 = Closed
  delay(150);
  animateEyes(0);   // Center
  delay(1000);

  // 2. Look Left, then Right, then Center
  animateEyes(2);   // 2 = Look Left
  delay(800);
  animateEyes(3);   // 3 = Look Right
  delay(800);
  animateEyes(0);   // Center
  delay(1000);

  // 3. Get Surprised
  animateMouth(1);  // 1 = Surprised "O"
  delay(2000);
  
  // 4. Back to Smiling
  animateMouth(0);  // Smile
}

// ---------------- DRAW BASE FACE ----------------
void drawDottedFaceBase() {
  // Draw only the yellow background dots
  for (int x = cx - radius; x <= cx + radius; x += DOT) {
    for (int y = cy - radius; y <= cy + radius; y += DOT) {
      int dx = x - cx;
      int dy = y - cy;
      if (dx * dx + dy * dy <= radius * radius) {
        tft.fillCircle(x, y, 2, ST77XX_YELLOW);
      }
    }
  }
}

// ---------------- EYE ANIMATOR ----------------
// States: 0=Center, 1=Closed, 2=Left, 3=Right
void animateEyes(int state) {
  // 1. Erase eye boxes (made slightly wider to handle looking sideways)
  tft.fillRect(cx - 22, cy - 15, 16, 14, ST77XX_BLACK); // Left box
  tft.fillRect(cx + 6, cy - 15, 16, 14, ST77XX_BLACK);  // Right box

  // 2. Patch background dots inside the eye boxes
  for (int x = cx - radius; x <= cx + radius; x += DOT) {
    for (int y = cy - radius; y <= cy + radius; y += DOT) {
      bool inLeftEye = (x >= cx - 22 && x <= cx - 6 && y >= cy - 15 && y <= cy - 1);
      bool inRightEye = (x >= cx + 6 && x <= cx + 22 && y >= cy - 15 && y <= cy - 1);

      if (inLeftEye || inRightEye) {
        tft.fillCircle(x, y, 2, ST77XX_YELLOW); 
      }
    }
  }

  // 3. Draw new eyes based on state
  if (state == 1) { // Closed
    tft.fillRect(cx - 18, cy - 10, 8, 2, ST77XX_BLACK);
    tft.fillRect(cx + 12, cy - 10, 8, 2, ST77XX_BLACK);
  } else { // Open (Center, Left, or Right)
    int offset = 0;
    if (state == 2) offset = -5; // Look Left
    if (state == 3) offset = 5;  // Look Right
    
    tft.fillCircle(cx - 15 + offset, cy - 10, 4, ST77XX_BLACK);
    tft.fillCircle(cx + 15 + offset, cy - 10, 4, ST77XX_BLACK);
  }
}

// ---------------- MOUTH ANIMATOR ----------------
// States: 0=Smile, 1=Surprised
void animateMouth(int state) {
  // 1. Erase the lower half of the face where the mouth lives
  tft.fillRect(cx - 25, cy + 5, 50, 22, ST77XX_BLACK);

  // 2. Patch background dots inside the mouth box
  for (int x = cx - radius; x <= cx + radius; x += DOT) {
    for (int y = cy - radius; y <= cy + radius; y += DOT) {
      bool inMouthArea = (x >= cx - 25 && x <= cx + 25 && y >= cy + 5 && y <= cy + 27);

      if (inMouthArea) {
        // Double check it's still inside the main face circle
        int dx = x - cx;
        int dy = y - cy;
        if (dx * dx + dy * dy <= radius * radius) {
          tft.fillCircle(x, y, 2, ST77XX_YELLOW); 
        }
      }
    }
  }

  // 3. Draw new mouth based on state
  if (state == 0) { // Smile
    for (int angle = 20; angle < 160; angle += 5) {
      int px = cx + 20 * cos(angle * 3.14 / 180);
      int py = cy + 20 * sin(angle * 3.14 / 180);
      tft.fillCircle(px, py, 2, ST77XX_BLACK);
    }
  } else if (state == 1) { // Surprised "O"
    // Draw an outline of a circle by drawing a slightly smaller yellow circle inside a black one
    tft.fillCircle(cx, cy + 15, 6, ST77XX_BLACK);
    tft.fillCircle(cx, cy + 15, 3, ST77XX_BLACK); // Solid black "O" looks better for this scale
  }
}