/*
  ============================================================================
  ePaper Weather Forecast Dashboard
  Board   : ESP32-S3 N16R8 (16MB Flash / 8MB Octal PSRAM)
  Display : WeAct Studio 1.54" ePaper Module (SSD1681, 200x200, GDEH0154D67)
  Data    : Open-Meteo API (free, no API key required)
  Library : GxEPD2 by ZinggJM  (install via Library Manager)
            ArduinoJson by Benoit Blanchon (install via Library Manager)
  ============================================================================

  WIRING (avoid GPIO 33-37 on N16R8 - reserved for octal PSRAM):

    EPD Pin   ->  ESP32-S3 GPIO
    ------------------------------
    VCC       ->  3V3
    GND       ->  GND
    DIN/MOSI  ->  GPIO 11
    CLK/SCK   ->  GPIO 12
    CS        ->  GPIO 10
    DC        ->  GPIO 9
    RST       ->  GPIO 8
    BUSY      ->  GPIO 7

  Change WIFI_SSID / WIFI_PASS and LAT / LON below before flashing.
  ============================================================================
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
// Note: Adafruit GFX only ships Free* fonts in 9/12/18/24pt sizes — there is
// no 6pt variant. Small text below uses the library's built-in bitmap font
// instead (activated by calling display.setFont() with no argument).

// ---------------------------------------------------------------------------
// USER CONFIG
// ---------------------------------------------------------------------------
const char* WIFI_SSID = "Airtel_Saerah_5G";
const char* WIFI_PASS = "Saerah@123";

// Default location: Kochi, Kerala. Change to your coordinates.
const float LAT = 9.9816;
const float LON = 76.2999;
const char* LOCATION_NAME = "Kochi, KL";

// Refresh interval (minutes). E-paper only draws power while refreshing,
// so this can safely be long to save power / reduce panel wear.
const uint32_t REFRESH_MINUTES = 30;

// Set true to deep-sleep between refreshes (best for battery power).
// Set false to keep WiFi/loop running (fine for USB-powered desk display).
const bool USE_DEEP_SLEEP = false;

// ---------------------------------------------------------------------------
// EPD PIN CONFIG
// ---------------------------------------------------------------------------
#define EPD_BUSY  7
#define EPD_RST   8
#define EPD_DC    9
#define EPD_CS    10
#define EPD_SCK   12
#define EPD_MOSI  11
#define EPD_MISO  -1   // not used by the display

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// Broad weather-icon category, used by codeToIcon()/drawIcon().
// NOTE: defined here (near the top) rather than further down, because the
// Arduino IDE auto-generates function prototypes and inserts them near the
// top of the file — if this enum were defined later, those generated
// prototypes would reference it before it exists and fail to compile.
enum IconType { ICON_SUN, ICON_CLOUD, ICON_PARTLY, ICON_RAIN, ICON_STORM, ICON_FOG, ICON_SNOW };

// ---------------------------------------------------------------------------
// WEATHER DATA STRUCT
// ---------------------------------------------------------------------------
struct DayForecast {
  String dayName;
  int weatherCode;
  float tMax;
  float tMin;
};

struct WeatherData {
  bool valid = false;
  float currentTemp = 0;
  float feelsLike = 0;
  int currentCode = 0;
  int humidity = 0;
  float windSpeed = 0;
  DayForecast days[3];
};

WeatherData wx;

// ---------------------------------------------------------------------------
// WMO weather code -> short label
// https://open-meteo.com/en/docs  (WMO Weather interpretation codes)
// ---------------------------------------------------------------------------
String codeToText(int code) {
  if (code == 0) return "Clear";
  if (code == 1 || code == 2) return "P.Cloudy";
  if (code == 3) return "Cloudy";
  if (code == 45 || code == 48) return "Fog";
  if (code >= 51 && code <= 57) return "Drizzle";
  if (code >= 61 && code <= 67) return "Rain";
  if (code >= 71 && code <= 77) return "Snow";
  if (code >= 80 && code <= 82) return "Showers";
  if (code >= 95) return "Storm";
  return "N/A";
}

IconType codeToIcon(int code) {
  if (code == 0) return ICON_SUN;
  if (code == 1 || code == 2) return ICON_PARTLY;
  if (code == 3) return ICON_CLOUD;
  if (code == 45 || code == 48) return ICON_FOG;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return ICON_RAIN;
  if (code >= 71 && code <= 77) return ICON_SNOW;
  if (code >= 95) return ICON_STORM;
  return ICON_CLOUD;
}

// ---------------------------------------------------------------------------
// ICON DRAWING (simple monochrome vector icons, small footprint for e-paper)
// ---------------------------------------------------------------------------
void drawSun(int cx, int cy, int r) {
  display.fillCircle(cx, cy, r, GxEPD_BLACK);
  for (int i = 0; i < 8; i++) {
    float a = i * PI / 4;
    int x1 = cx + (r + 3) * cos(a);
    int y1 = cy + (r + 3) * sin(a);
    int x2 = cx + (r + 8) * cos(a);
    int y2 = cy + (r + 8) * sin(a);
    display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  }
}

void drawCloud(int cx, int cy, int scale) {
  display.fillCircle(cx - scale, cy, scale * 0.7, GxEPD_BLACK);
  display.fillCircle(cx + scale * 0.3, cy - scale * 0.4, scale * 0.9, GxEPD_BLACK);
  display.fillCircle(cx + scale, cy + scale * 0.1, scale * 0.6, GxEPD_BLACK);
  display.fillRect(cx - scale, cy, scale * 2.2, scale * 0.7, GxEPD_BLACK);
}

void drawRain(int cx, int cy, int scale) {
  drawCloud(cx, cy - scale * 0.5, scale * 0.8);
  for (int i = -1; i <= 1; i++) {
    int x = cx + i * scale * 0.6;
    display.drawLine(x, cy + scale * 0.4, x - 3, cy + scale * 1.1, GxEPD_BLACK);
  }
}

void drawStorm(int cx, int cy, int scale) {
  drawCloud(cx, cy - scale * 0.5, scale * 0.8);
  int x0 = cx + 4, y0 = cy + scale * 0.3;
  display.fillTriangle(x0, y0, x0 - 8, y0 + 14, x0 + 2, y0 + 6, GxEPD_BLACK);
  display.fillTriangle(x0 + 2, y0 + 6, x0 - 4, y0 + 22, x0 + 6, y0 + 10, GxEPD_BLACK);
}

void drawSnow(int cx, int cy, int scale) {
  drawCloud(cx, cy - scale * 0.5, scale * 0.8);
  for (int i = -1; i <= 1; i++) {
    int x = cx + i * scale * 0.6;
    int y = cy + scale * 0.9;
    display.drawLine(x - 4, y, x + 4, y, GxEPD_BLACK);
    display.drawLine(x, y - 4, x, y + 4, GxEPD_BLACK);
  }
}

void drawFog(int cx, int cy, int scale) {
  for (int i = -1; i <= 1; i++) {
    display.drawFastHLine(cx - scale, cy + i * 6, scale * 2, GxEPD_BLACK);
    display.drawFastHLine(cx - scale, cy + i * 6 + 1, scale * 2, GxEPD_BLACK);
  }
}

void drawIcon(IconType type, int cx, int cy, int scale) {
  switch (type) {
    case ICON_SUN:    drawSun(cx, cy, scale); break;
    case ICON_CLOUD:  drawCloud(cx, cy, scale); break;
    case ICON_PARTLY:
      drawSun(cx - scale * 0.4, cy - scale * 0.3, scale * 0.55);
      drawCloud(cx + scale * 0.2, cy + scale * 0.2, scale * 0.7);
      break;
    case ICON_RAIN:   drawRain(cx, cy, scale); break;
    case ICON_STORM:  drawStorm(cx, cy, scale); break;
    case ICON_SNOW:   drawSnow(cx, cy, scale); break;
    case ICON_FOG:    drawFog(cx, cy, scale); break;
  }
}

// ---------------------------------------------------------------------------
// WIFI
// ---------------------------------------------------------------------------
bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected, IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("WiFi connection failed");
  return false;
}

// ---------------------------------------------------------------------------
// FETCH WEATHER (Open-Meteo, no API key needed)
// ---------------------------------------------------------------------------
bool fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(LAT, 4) +
               "&longitude=" + String(LON, 4) +
               "&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m" +
               "&daily=weather_code,temperature_2m_max,temperature_2m_min" +
               "&timezone=auto&forecast_days=4";

  http.begin(url);
  int httpCode = http.GET();
  if (httpCode != 200) {
    Serial.printf("HTTP GET failed, code: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Filter to keep only the JSON fields we need (reduces RAM use)
  JsonDocument filter;
  filter["current"]["temperature_2m"] = true;
  filter["current"]["relative_humidity_2m"] = true;
  filter["current"]["apparent_temperature"] = true;
  filter["current"]["weather_code"] = true;
  filter["current"]["wind_speed_10m"] = true;
  filter["daily"]["time"] = true;
  filter["daily"]["weather_code"] = true;
  filter["daily"]["temperature_2m_max"] = true;
  filter["daily"]["temperature_2m_min"] = true;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (err) {
    Serial.print("JSON parse failed: ");
    Serial.println(err.c_str());
    return false;
  }

  wx.currentTemp = doc["current"]["temperature_2m"] | 0.0;
  wx.feelsLike   = doc["current"]["apparent_temperature"] | 0.0;
  wx.humidity    = doc["current"]["relative_humidity_2m"] | 0;
  wx.windSpeed   = doc["current"]["wind_speed_10m"] | 0.0;
  wx.currentCode = doc["current"]["weather_code"] | 0;

  const char* shortDays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

  // daily[0] is today; use indices 1..3 for the next 3 days
  for (int i = 0; i < 3; i++) {
    int dIdx = i + 1;
    String dateStr = doc["daily"]["time"][dIdx] | "";
    String dayLabel = "Day" + String(i + 1);
    if (dateStr.length() == 10) {
      // dateStr format YYYY-MM-DD -> compute weekday
      int y = dateStr.substring(0, 4).toInt();
      int m = dateStr.substring(5, 7).toInt();
      int d = dateStr.substring(8, 10).toInt();
      // Zeller-ish simple day-of-week calc (Sakamoto's algorithm)
      static const int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
      if (m < 3) y -= 1;
      int w = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
      dayLabel = shortDays[w];
    }
    wx.days[i].dayName = dayLabel;
    wx.days[i].weatherCode = doc["daily"]["weather_code"][dIdx] | 0;
    wx.days[i].tMax = doc["daily"]["temperature_2m_max"][dIdx] | 0.0;
    wx.days[i].tMin = doc["daily"]["temperature_2m_min"][dIdx] | 0.0;
  }

  wx.valid = true;
  return true;
}

// ---------------------------------------------------------------------------
// RENDER DASHBOARD TO EPAPER (200 x 200)
// ---------------------------------------------------------------------------
void drawDashboard() {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // --- Header bar ---
    display.drawFastHLine(0, 22, 200, GxEPD_BLACK);
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(4, 16);
    display.print(LOCATION_NAME);

    if (!wx.valid) {
      display.setFont(&FreeSans9pt7b);
      display.setCursor(10, 100);
      display.print("Weather data");
      display.setCursor(10, 118);
      display.print("unavailable");
      display.setCursor(10, 140);
      display.print("Check WiFi / API");
    } else {
      // --- Main current temp + icon ---
      drawIcon(codeToIcon(wx.currentCode), 45, 65, 24);

      display.setFont(&FreeSansBold18pt7b);
      display.setCursor(95, 75);
      display.print(String((int)round(wx.currentTemp)) + "C");

      display.setFont(&FreeSans9pt7b);
      display.setCursor(95, 95);
      display.print(codeToText(wx.currentCode));

      // --- Feels like / humidity / wind row ---
      // Built-in bitmap font: setCursor is top-left (not baseline like the
      // Free* fonts), so this row sits a touch higher than the others.
      display.setFont();
      display.setTextSize(1);
      display.setCursor(4, 102);
      display.print("Feels " + String((int)round(wx.feelsLike)) + "C");
      display.setCursor(70, 102);
      display.print("Hum " + String(wx.humidity) + "%");
      display.setCursor(130, 102);
      display.print("Wind " + String((int)round(wx.windSpeed)) + "km/h");

      display.drawFastHLine(0, 116, 200, GxEPD_BLACK);

      // --- 3-day forecast row ---
      int colW = 200 / 3;
      for (int i = 0; i < 3; i++) {
        int cx = colW * i + colW / 2;

        display.setFont(&FreeSansBold9pt7b);
        int16_t bx, by; uint16_t bw, bh;
        display.getTextBounds(wx.days[i].dayName, 0, 0, &bx, &by, &bw, &bh);
        display.setCursor(cx - bw / 2, 130);
        display.print(wx.days[i].dayName);

        drawIcon(codeToIcon(wx.days[i].weatherCode), cx, 155, 14);

        display.setFont();
        display.setTextSize(1);
        String temps = String((int)round(wx.days[i].tMax)) + "/" +
                       String((int)round(wx.days[i].tMin));
        display.getTextBounds(temps, 0, 0, &bx, &by, &bw, &bh);
        display.setCursor(cx - bw / 2, 183);
        display.print(temps);

        if (i > 0) display.drawFastVLine(colW * i, 118, 78, GxEPD_BLACK);
      }
    }
  } while (display.nextPage());
}

// ---------------------------------------------------------------------------
// SETUP / LOOP
// ---------------------------------------------------------------------------
void updateDashboard() {
  if (connectWiFi()) {
    fetchWeather();
    WiFi.disconnect(true);
  }
  drawDashboard();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI, EPD_CS);
  display.init(115200, true, 2, false);
  display.setRotation(2);

  updateDashboard();

  if (USE_DEEP_SLEEP) {
    display.hibernate();
    Serial.printf("Sleeping for %u minutes...\n", REFRESH_MINUTES);
    esp_sleep_enable_timer_wakeup((uint64_t)REFRESH_MINUTES * 60ULL * 1000000ULL);
    esp_deep_sleep_start();
  }
}

void loop() {
  if (!USE_DEEP_SLEEP) {
    static uint32_t lastUpdate = 0;
    uint32_t intervalMs = REFRESH_MINUTES * 60UL * 1000UL;
    if (millis() - lastUpdate >= intervalMs || lastUpdate == 0) {
      lastUpdate = millis();
      updateDashboard();
    }
    delay(1000);
  }
}
