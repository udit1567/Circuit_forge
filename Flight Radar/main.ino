#include <SPI.h>
 #include "Adafruit_GFX.h"
 #include "Adafruit_GC9A01A.h"
 #include <WiFi.h>
 #include <HTTPClient.h>
 #include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
 
 // ── User config ──────────────────────────────────────────────
 #define WIFI_SSID             "Infinity"
 #define WIFI_PASSWORD         "#Udit1588"
// GPS overrides radar centre at runtime once fix acquired
 #define RADAR_LAT_DEFAULT      25.3176f   // Varanasi fallback
 #define RADAR_LON_DEFAULT      83.0128f
 #define GPS_RX_PIN  20
 #define GPS_TX_PIN  21
 #define GPS_BAUD    9600

TinyGPSPlus  gps;
HardwareSerial gpsSerial(1);

// Live radar centre — updated from GPS when fix valid
static float radarLat = RADAR_LAT_DEFAULT;
static float radarLon = RADAR_LON_DEFAULT;
static bool  gpsFixed     = false;
static int   gpsSats      = 0;
static bool  gpsFirstFix    = false;   // Issue 3: first-fix flag
 #define OPENSKY_CLIENT_ID     "ckt.forge@gmail.com-api-client"
 #define OPENSKY_CLIENT_SECRET "HRy7EAnbZ9G8rc0kqgGWqFOzv0RcOSdM"
 
 // ── Radar tuning ─────────────────────────────────────────────
 #define RADAR_RAD        3.0f
 #define RADAR_FETCH_MS   30000UL
 #define RADAR_MAX_AC     20
 #define SWEEP_STEP       2.0f
 #define FRAME_MS         30
 
 // ── Display ──────────────────────────────────────────────────
 #define TFT_CS   10
 #define TFT_DC    4
 #define TFT_RST   5
 #define TFT_MOSI  7
 #define TFT_SCLK  6
 
 Adafruit_GC9A01A tft(&SPI, TFT_DC, TFT_CS, TFT_RST);
 
 // ── Encoder ──────────────────────────────────────────────────
 #define ENC_CLK  1
 #define ENC_DT   2
 #define ENC_SW   3
 
 volatile int encDelta  = 0;
 static int   encLastCLK = HIGH;
 
 void IRAM_ATTR encoderISR() {
   int clk = digitalRead(ENC_CLK);
   if (clk != encLastCLK && clk == LOW)
     encDelta += (digitalRead(ENC_DT) == HIGH) ? 1 : -1;
   encLastCLK = clk;
 }
 
 static int readEncoder() {
   noInterrupts(); int d = encDelta; encDelta = 0; interrupts();
   return d;
 }
 
 // Button — returns true on SHORT press release
 static bool swLastState  = HIGH;
 static unsigned long swPressAt = 0;
 static bool readButton() {
   bool cur = (digitalRead(ENC_SW) == LOW);
   if (cur  && !swLastState) swPressAt = millis();
   if (!cur &&  swLastState) { swLastState = false; return (millis() - swPressAt < 800); }
   swLastState = cur;
   return false;
 }
 
 // ── Colours ──────────────────────────────────────────────────
 static inline uint16_t C(uint8_t r, uint8_t g, uint8_t b) {
   return ((uint16_t)(r >> 3) << 11) | ((uint16_t)(g >> 2) << 5) | (b >> 3);
 }
 #define COL_BG        GC9A01A_BLACK
 #define COL_SWEEP     C(0, 255, 0)
 #define COL_RING_OUT  C(0, 180, 0)
 #define COL_RING_MID  C(0,  60, 0)
 #define COL_RING_IN   C(0,  30, 0)
 #define COL_CROSS     C(0,  40, 0)
 #define COL_LABEL     C(0, 150, 0)
 #define COL_SEL       GC9A01A_YELLOW      // selection ring
 #define COL_PANEL_BG  C(10, 20, 10)       // detail panel background
 #define COL_PANEL_BDR GC9A01A_YELLOW      // detail panel border
 #define COL_CYAN      C(0, 255, 255)
 #define COL_WHITE     GC9A01A_WHITE
 #define COL_DIM       C(120, 120, 120)
 
 static const uint16_t AC_PAL[] = {
   C(0,255,128), C(0,220,255), C(0,128,255), C(100,200,255),
   C(160,130,255), C(220,80,255), C(255,50,150), C(255,190,0)
 };
 #define AC_PAL_SZ (int)(sizeof(AC_PAL)/sizeof(AC_PAL[0]))
 
 static uint16_t acColor(const char* s) {
   uint32_t h = 0; for (; *s; s++) h = h * 31 + (uint8_t)*s;
   return AC_PAL[h % AC_PAL_SZ];
 }
 
 // ── Radar geometry ───────────────────────────────────────────
 #define RCX    120
 #define RCY    120
 #define ROUTER 100   // Issue 2: reduced from 110 — status bar at y=212 clears ring bottom at y=220
 
 // ── Aircraft store ───────────────────────────────────────────
 struct RadarAC {
   char     callsign[10];
   char     icao[8];
   char     country[24];
   float    lat, lon, heading;
   float    alt_m;       // baro altitude metres  (-1 = unknown)
   float    vel_ms;      // velocity m/s           (-1 = unknown)
   int      px, py, opx, opy;
   uint16_t color;
 };
 static RadarAC   radarAC[RADAR_MAX_AC];
 static int       radarCount       = 0;
 static float     sweepAngle       = 0.0f;
 static float     sweepOldAngle    = 0.0f;
 static unsigned long lastFetchMs  = 0;
 static unsigned long lastFrameMs  = 0;
 
 // ── Selection state ──────────────────────────────────────────
 static int  selIdx     = 0;   // currently selected aircraft index
 static int  prevSelIdx = -1;  // previously drawn selection (for erase)
 static bool detailOpen = false;
 
 // -- Boot mode -------------------------------------------------
#define BOOT_MODE_RADAR  0
#define BOOT_MODE_GPS    1
static int  bootMode = BOOT_MODE_RADAR;

static void drawModeSelect(int sel) {
  tft.fillScreen(COL_BG);
  tft.setTextColor(COL_WHITE, COL_BG); tft.setTextSize(1);
  tft.setCursor(75, 70); tft.print("SELECT MODE");
  tft.drawFastHLine(50, 82, 140, COL_RING_MID);

  if (sel == 0) tft.fillRoundRect(52, 94, 136, 22, 5, C(0,40,0));
  tft.setTextColor(sel == 0 ? COL_SEL : COL_DIM, sel == 0 ? C(0,40,0) : COL_BG);
  if (sel == 0) tft.drawRoundRect(52, 94, 136, 22, 5, COL_RING_OUT);
  tft.setCursor(70, 100); tft.print("Flight Radar");

  if (sel == 1) tft.fillRoundRect(52, 122, 136, 22, 5, C(0,40,0));
  tft.setTextColor(sel == 1 ? COL_SEL : COL_DIM, sel == 1 ? C(0,40,0) : COL_BG);
  if (sel == 1) tft.drawRoundRect(52, 122, 136, 22, 5, COL_RING_OUT);
  tft.setCursor(76, 128); tft.print("GPS Status");

  tft.setTextColor(C(60,60,60), COL_BG);
  tft.setCursor(54, 158); tft.print("Turn=scroll  SW=select");
  tft.setCursor(68, 170); tft.print("Auto: Radar in 8s");
}

static void runGpsStatusScreen() {
  while (true) {
    // Feed GPS
    while (gpsSerial.available()) gps.encode(gpsSerial.read());

    bool fix = gps.location.isValid() &&
               gps.location.age() < 3000 &&
               gps.hdop.isValid() && gps.hdop.hdop() < 5.0f &&
               gps.satellites.isValid() && gps.satellites.value() >= 4;

    // ── Full screen redraw (1 s refresh, no flicker on round display) ──
    tft.fillScreen(COL_BG);

    // Outer ring decoration
    tft.drawCircle(120, 120, 118, COL_RING_IN);

    // ── Header bar ───────────────────────────────────────────────────
    tft.fillRect(30, 18, 180, 18, C(0,30,0));
    tft.drawRoundRect(30, 18, 180, 18, 4, COL_RING_OUT);
    tft.setTextColor(COL_WHITE, C(0,30,0)); tft.setTextSize(1);
    tft.setCursor(70, 23); tft.print("GPS STATUS");

    // ── Fix badge ────────────────────────────────────────────────────
    uint16_t fixCol = fix ? C(0,200,0) : C(200,0,0);
    tft.fillRoundRect(155, 19, 50, 15, 3, fixCol);
    tft.setTextColor(COL_BG, fixCol);
    tft.setCursor(159, 23); tft.print(fix ? "FIX OK" : "NO FIX");

    // ── Satellite count (large) ──────────────────────────────────────
    tft.setTextSize(3);
    tft.setTextColor(GC9A01A_YELLOW, COL_BG);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", (int)gps.satellites.value());
    tft.setCursor(100, 42); tft.print(buf);
    tft.setTextSize(1);
    tft.setTextColor(COL_DIM, COL_BG);
    tft.setCursor(100, 70); tft.print("satellites");

    // Divider
    tft.drawFastHLine(20, 80, 200, COL_RING_MID);

    // ── Data rows ────────────────────────────────────────────────────
    struct GpsRow { const char* label; char val[28]; uint16_t col; };
    GpsRow rows[8];

    // LAT
    rows[0].label = "LAT";
    rows[0].col   = COL_CYAN;
    if (gps.location.isValid())
      snprintf(rows[0].val, 28, "%.6f N", gps.location.lat());
    else strcpy(rows[0].val, "---");

    // LON
    rows[1].label = "LON";
    rows[1].col   = COL_CYAN;
    if (gps.location.isValid())
      snprintf(rows[1].val, 28, "%.6f E", gps.location.lng());
    else strcpy(rows[1].val, "---");

    // ALTITUDE
    rows[2].label = "ALT";
    rows[2].col   = C(255,200,0);
    if (gps.altitude.isValid())
      snprintf(rows[2].val, 28, "%.1fm / %.0fft",
               gps.altitude.meters(), gps.altitude.feet());
    else strcpy(rows[2].val, "---");

    // SPEED
    rows[3].label = "SPEED";
    rows[3].col   = C(0,220,120);
    if (gps.speed.isValid())
      snprintf(rows[3].val, 28, "%.1f km/h  %.0f kts",
               gps.speed.kmph(), gps.speed.knots());
    else strcpy(rows[3].val, "---");

    // HEADING
    rows[4].label = "HDG";
    rows[4].col   = COL_WHITE;
    if (gps.course.isValid())
      snprintf(rows[4].val, 28, "%.1f deg", gps.course.deg());
    else strcpy(rows[4].val, "---");

    // HDOP
    rows[5].label = "HDOP";
    rows[5].col   = (gps.hdop.isValid() && gps.hdop.hdop() < 2.0f) ? C(0,220,0) :
                    (gps.hdop.isValid() && gps.hdop.hdop() < 5.0f) ? GC9A01A_YELLOW : C(220,0,0);
    if (gps.hdop.isValid())
      snprintf(rows[5].val, 28, "%.2f", gps.hdop.hdop());
    else strcpy(rows[5].val, "---");

    // TIME IST (UTC+5:30)
    rows[6].label = "TIME IST";
    rows[6].col   = C(255,160,0);
    if (gps.time.isValid()) {
      int hr = gps.time.hour() + 5;
      int mn = gps.time.minute() + 30;
      if (mn >= 60) { mn -= 60; hr++; }
      if (hr >= 24)  hr -= 24;
      snprintf(rows[6].val, 28, "%02d:%02d:%02d", hr, mn, gps.time.second());
    } else strcpy(rows[6].val, "---");

    // DATE
    rows[7].label = "DATE";
    rows[7].col   = C(255,160,0);
    if (gps.date.isValid())
      snprintf(rows[7].val, 28, "%04d-%02d-%02d",
               gps.date.year(), gps.date.month(), gps.date.day());
    else strcpy(rows[7].val, "---");

    // Render rows
    tft.setTextSize(1);
    for (int i = 0; i < 8; i++) {
      int y = 86 + i * 17;
      tft.setTextColor(COL_RING_OUT, COL_BG);
      tft.setCursor(22, y); tft.print(rows[i].label); tft.print(":");
      tft.setTextColor(rows[i].col, COL_BG);
      tft.setCursor(80, y); tft.print(rows[i].val);
    }

    // Footer
    tft.drawFastHLine(20, 224, 200, COL_RING_MID);
    tft.setTextColor(C(50,50,50), COL_BG);
    tft.setCursor(30, 227);
    tft.printf("chars:%lu", gps.charsProcessed());

    delay(1000);
  }
}

// ── OAuth2 ───────────────────────────────────────────────────
 static char          oauthToken[2048] = "";
 static unsigned long tokenExpireMs    = 0;
 
 #define TOKEN_URL \
   "https://auth.opensky-network.org/auth/realms/opensky-network" \
   "/protocol/openid-connect/token"
 
 // ── Coordinate helpers ───────────────────────────────────────
 static void latLonToXY(float lat, float lon, int &x, int &y) {
   // Map lat/lon to normalised offsets from radar centre (-1..+1)
   float fx = (lon - radarLon) / RADAR_RAD;   // +1 = east edge, -1 = west edge
   float fy = (lat - radarLat) / RADAR_RAD;   // +1 = north edge, -1 = south edge

   // Scale to radar pixel radius, inset by 8 px so triangle/circle fits inside ring
   const float maxR = ROUTER - 8.0f;
   float px = fx * maxR;
   float py = -fy * maxR;   // screen Y increases downward

   // Clamp to radar circle — push any out-of-range blip onto the ring edge
   float dist = sqrtf(px * px + py * py);
   if (dist > maxR && dist > 0.0f) {
     px = px / dist * maxR;
     py = py / dist * maxR;
   }

   x = RCX + (int)px;
   y = RCY + (int)py;
 }
 
 // ── Background elements ──────────────────────────────────────
 static void drawRings() {
   tft.drawCircle(RCX, RCY, ROUTER,       COL_RING_OUT);
   tft.drawCircle(RCX, RCY, ROUTER * 2/3, COL_RING_MID);
   tft.drawCircle(RCX, RCY, ROUTER / 3,   COL_RING_IN);
 }
 static void drawCross() {
   tft.drawFastHLine(RCX - ROUTER, RCY, ROUTER * 2 + 1, COL_CROSS);
   tft.drawFastVLine(RCX, RCY - ROUTER, ROUTER * 2 + 1, COL_CROSS);
 }
 static void drawBackground() {
   tft.fillScreen(COL_BG);
   drawRings(); drawCross();
   tft.setTextColor(COL_LABEL, COL_BG); tft.setTextSize(1);
   tft.setCursor(RCX - 3, RCY - ROUTER + 3);  tft.print("N");
   tft.setCursor(RCX - 3, RCY + ROUTER - 11); tft.print("S");
   tft.setCursor(RCX + ROUTER - 8, RCY - 4);  tft.print("E");
   tft.setCursor(RCX - ROUTER + 2, RCY - 4);  tft.print("W");
   tft.fillCircle(RCX, RCY, 2, COL_SWEEP);
 }
 
 // ── Aircraft draw helpers ─────────────────────────────────────
 static void drawAC(int x, int y, float hdg, uint16_t col, bool erase) {
   uint16_t c = erase ? COL_BG : col;
   float hr = hdg * DEG_TO_RAD;
   float dx = sinf(hr), dy = -cosf(hr);
   float px = -dy,      py =  dx;
   tft.drawLine(x + (int)(dx*7), y + (int)(dy*7),
                x - (int)(dx*5), y - (int)(dy*5), c);
   int wx = x + (int)(dx*2), wy = y + (int)(dy*2);
   tft.fillTriangle(wx, wy,
     wx + (int)(px*7 - dx*3), wy + (int)(py*7 - dy*3),
     wx - (int)(px*7 + dx*3), wy - (int)(py*7 + dy*3), c);
 }
 
 // Draw selection ring around aircraft i.  erase=true → draw BG colour.
 static void drawSelRing(int i, bool erase) {
   if (i < 0 || i >= radarCount) return;
   uint16_t col = erase ? COL_BG : COL_SEL;
   tft.drawCircle(radarAC[i].px, radarAC[i].py, 11, col);
   tft.drawCircle(radarAC[i].px, radarAC[i].py, 12, col);
   // Restore any radar rings the selection ring may have erased
   if (!erase) return;
   for (int r : {ROUTER, ROUTER*2/3, ROUTER/3}) {
     int dx = radarAC[i].px - RCX, dy = radarAC[i].py - RCY;
     int dist = (int)sqrtf((float)(dx*dx + dy*dy));
     if (abs(dist - r) <= 13) tft.drawCircle(RCX, RCY, r,
       r == ROUTER ? COL_RING_OUT : (r == ROUTER*2/3 ? COL_RING_MID : COL_RING_IN));
   }
   // Restore crosshair pixels near the blip
   if (abs(radarAC[i].py - RCY) <= 13)
     tft.drawFastHLine(RCX - ROUTER, RCY, ROUTER*2+1, COL_CROSS);
   if (abs(radarAC[i].px - RCX) <= 13)
     tft.drawFastVLine(RCX, RCY - ROUTER, ROUTER*2+1, COL_CROSS);
 }
 
 // Draw a callsign tag below the selected aircraft
 static void drawSelTag(int i, bool erase) {
   if (i < 0 || i >= radarCount) return;
   int tx = radarAC[i].px - 18, ty = radarAC[i].py + 14;
   // Clamp to screen
   if (tx < 2)   tx = 2;
   if (tx > 196) tx = 196;
   if (ty > 228) ty = 228;
   tft.fillRect(tx, ty, 44, 9, erase ? COL_BG : C(0,20,0));
   if (!erase) {
     tft.setTextColor(COL_SEL, C(0,20,0));
     tft.setTextSize(1);
     tft.setCursor(tx + 1, ty + 1);
     tft.print(radarAC[i].callsign);
   }
 }
 
 static void drawAllAC() {
   for (int i = 0; i < radarCount; i++)
     drawAC(radarAC[i].px, radarAC[i].py, radarAC[i].heading, radarAC[i].color, false);
 }
 
 // Redraw selection ring + tag for current selection (differential)
 static void updateSelection(int oldIdx, int newIdx) {
   // Erase old selection
   if (oldIdx >= 0 && oldIdx < radarCount) {
     drawSelRing(oldIdx, true);           // erase ring → restores BG
     drawSelTag(oldIdx, true);
     drawAC(radarAC[oldIdx].px, radarAC[oldIdx].py,   // redraw blip in plain colour
            radarAC[oldIdx].heading, radarAC[oldIdx].color, false);
   }
   // Draw new selection
   if (newIdx >= 0 && newIdx < radarCount) {
     drawSelRing(newIdx, false);
     drawAC(radarAC[newIdx].px, radarAC[newIdx].py,   // redraw blip on top of ring
            radarAC[newIdx].heading, radarAC[newIdx].color, false);
     drawSelTag(newIdx, false);
   }
 }
 
// ── Detail panel — full screen, matches reference screenshot ──
static void drawDetailPanel() {
  if (radarCount == 0) return;
  RadarAC &a = radarAC[selIdx];

  // Full screen wipe — clean slate like reference
  tft.fillScreen(COL_BG);

  // Outer green circle border (matches reference)
  tft.drawCircle(120, 120, 118, COL_RING_OUT);
  tft.drawCircle(120, 120, 117, C(0,60,0));

  // ── Plane icon at top (arrow pointing in heading direction) ──
  int ix = 120, iy = 38;
  float hr = a.heading * DEG_TO_RAD;
  float ddx = sinf(hr), ddy = -cosf(hr);
  float ppx = -ddy,     ppy =  ddx;
  // fuselage
  tft.drawLine(ix + (int)(ddx*12), iy + (int)(ddy*12),
               ix - (int)(ddx*10), iy - (int)(ddy*10), COL_SWEEP);
  // wings
  int wx = ix + (int)(ddx*2), wy = iy + (int)(ddy*2);
  tft.fillTriangle(wx, wy,
    wx + (int)(ppx*11 - ddx*4), wy + (int)(ppy*11 - ddy*4),
    wx - (int)(ppx*11 + ddx*4), wy - (int)(ppy*11 + ddy*4), COL_SWEEP);
  // tail
  tft.fillTriangle(ix - (int)(ddx*7), iy - (int)(ddy*7),
    ix - (int)(ddx*10) + (int)(ppx*5), iy - (int)(ddy*10) + (int)(ppy*5),
    ix - (int)(ddx*10) - (int)(ppx*5), iy - (int)(ddy*10) - (int)(ppy*5), COL_SWEEP);

  // ── Callsign — large centred ─────────────────────────────
  const char* cs = a.callsign[0] ? a.callsign : a.icao;
  tft.setTextSize(3);
  tft.setTextColor(COL_SWEEP, COL_BG);
  int csLen = (int)strlen(cs);
  int csX   = 120 - (csLen * 9);
  if (csX < 20) csX = 20;
  tft.setCursor(csX, 60);
  tft.print(cs);

  // Thin divider
  tft.drawFastHLine(40, 88, 160, C(0,60,0));

  // ── Data rows — green monospace, reference style ─────────
  tft.setTextSize(1);
  char buf[32];

  // ALT — label green, value cyan
  tft.setTextColor(COL_SWEEP, COL_BG);
  tft.setCursor(44, 100);
  tft.print("ALT ");
  tft.setTextColor(GC9A01A_CYAN, COL_BG);
  if (a.alt_m >= 0) snprintf(buf, 32, "%d m", (int)a.alt_m);
  else strcpy(buf, "--- m");
  tft.print(buf);

  // SPD — label green, value yellow
  tft.setTextColor(COL_SWEEP, COL_BG);
  tft.setCursor(44, 118);
  tft.print("SPD ");
  tft.setTextColor(GC9A01A_YELLOW, COL_BG);
  if (a.vel_ms >= 0) snprintf(buf, 32, "%d m/s", (int)a.vel_ms);
  else strcpy(buf, "--- m/s");
  tft.print(buf);

  // HDG — label green, value orange
  tft.setTextColor(COL_SWEEP, COL_BG);
  tft.setCursor(44, 136);
  tft.print("HDG ");
  tft.setTextColor(C(255,140,0), COL_BG);
  snprintf(buf, 32, "%d deg", (int)a.heading);
  tft.print(buf);

  // COUNTRY — label green, value white
  tft.setTextColor(COL_SWEEP, COL_BG);
  tft.setCursor(44, 154);
  tft.print("COUNTRY ");
  tft.setTextColor(GC9A01A_WHITE, COL_BG);
  tft.print(a.country[0] ? a.country : "---");

  // ICAO24 — label green, value magenta
  tft.setTextColor(COL_SWEEP, COL_BG);
  tft.setCursor(44, 172);
  tft.print("ICAO ");
  tft.setTextColor(GC9A01A_MAGENTA, COL_BG);
  tft.print(a.icao[0] ? a.icao : "------");

  // Divider
  tft.drawFastHLine(40, 186, 160, C(0,60,0));

  // ── Bottom hints ─────────────────────────────────────────
  tft.setTextColor(C(0,140,0), COL_BG);
  snprintf(buf, 32, "%d/%d", selIdx + 1, radarCount);
  tft.setCursor(44, 196); tft.print(buf);

  tft.setCursor(64, 214);
  tft.print("< Rotate >");
}

static void closeDetailPanel() {
  // detail card used fillScreen so full redraw needed
  drawBackground();
  drawAllAC();
  if (radarCount > 0 && selIdx < radarCount) {
    drawSelRing(selIdx, false);
    drawAC(radarAC[selIdx].px, radarAC[selIdx].py,
           radarAC[selIdx].heading, radarAC[selIdx].color, false);
    drawSelTag(selIdx, false);
  }
}

 
 // ── OAuth2 ───────────────────────────────────────────────────
 static bool fetchToken() {
   Serial.println(F("[Auth] Fetching OAuth2 token..."));  Serial.flush();
   HTTPClient http;
   http.begin(TOKEN_URL);
   http.addHeader("Content-Type", "application/x-www-form-urlencoded");
   http.setTimeout(12000);
   String body = F("grant_type=client_credentials&client_id=");
   body += OPENSKY_CLIENT_ID;  body += F("&client_secret=");  body += OPENSKY_CLIENT_SECRET;
   int code = http.POST(body);
   Serial.printf("[Auth] HTTP %d\n", code);  Serial.flush();
   if (code != 200) { http.end(); return false; }
   String resp = http.getString();  http.end();
   DynamicJsonDocument doc(4096);
   if (deserializeJson(doc, resp) != DeserializationError::Ok) {
     Serial.println(F("[Auth] JSON parse FAILED"));  Serial.flush();  return false;
   }
   const char* tok = doc["access_token"];
   if (!tok || !strlen(tok)) { Serial.println(F("[Auth] No token")); Serial.flush(); return false; }
   strncpy(oauthToken, tok, sizeof(oauthToken) - 1);
   oauthToken[sizeof(oauthToken) - 1] = '\0';
   unsigned long exp = doc["expires_in"] | 300UL;
   tokenExpireMs = millis() + (exp - 30UL) * 1000UL;
   Serial.printf("[Auth] Token OK, %lu s, len=%d\n", exp, (int)strlen(oauthToken));  Serial.flush();
   return true;
 }
 static bool ensureToken() {
   if (!strlen(oauthToken) || millis() >= tokenExpireMs) return fetchToken();
   return true;
 }
 
 // ── Aircraft fetch ───────────────────────────────────────────
 static void fetchAircraft() {
   if (!ensureToken()) { Serial.println(F("[Radar] No token")); return; }
   char url[300];
   snprintf(url, sizeof(url),
     "https://opensky-network.org/api/states/all"
     "?lamin=%.4f&lomin=%.4f&lamax=%.4f&lomax=%.4f",
     radarLat - RADAR_RAD, radarLon - RADAR_RAD,
     radarLat + RADAR_RAD, radarLon + RADAR_RAD);
   Serial.println(url);  Serial.flush();
 
   HTTPClient http;
   http.begin(url);  http.setTimeout(15000);
   String authHdr = "Bearer "; authHdr += oauthToken;
   http.addHeader("Authorization", authHdr);
   int code = http.GET();
   Serial.printf("[Radar] HTTP %d\n", code);  Serial.flush();
   if (code == 401) { oauthToken[0] = '\0'; tokenExpireMs = 0; http.end(); return; }
   if (code != 200) { http.end(); return; }
   String resp = http.getString();  http.end();
   Serial.printf("[Radar] Response len: %d\n", resp.length());  Serial.flush();
 
   // Erase old blips
   for (int i = 0; i < radarCount; i++)
     drawAC(radarAC[i].opx, radarAC[i].opy, radarAC[i].heading, 0, true);
 
   DynamicJsonDocument doc(40960);
   if (deserializeJson(doc, resp) != DeserializationError::Ok) {
     Serial.println(F("[Radar] JSON error"));  Serial.flush();  return;
   }
 
   radarCount = 0;
   JsonArray states = doc["states"].as<JsonArray>();
   Serial.printf("[Radar] States: %d\n", (int)states.size());  Serial.flush();
 
   for (JsonVariant sv : states) {
     if (radarCount >= RADAR_MAX_AC) break;
     if (sv[5].isNull() || sv[6].isNull()) continue;
     float lon = sv[5].as<float>();
     float lat = sv[6].as<float>();
     if (lat == 0.0f && lon == 0.0f) continue;
 
     RadarAC &ac = radarAC[radarCount];
 
     // ICAO24
     const char *ic = sv[0].isNull() ? "" : sv[0].as<const char*>();
     strncpy(ac.icao, ic, 7);  ac.icao[7] = '\0';
 
     // Callsign
     const char *cs = sv[1].isNull() ? "" : sv[1].as<const char*>();
     strncpy(ac.callsign, cs, 9);  ac.callsign[9] = '\0';
     for (int k = (int)strlen(ac.callsign)-1; k >= 0 && ac.callsign[k]==' '; k--) ac.callsign[k]='\0';
     if (!strlen(ac.callsign)) { strncpy(ac.callsign, ac.icao, 9); ac.callsign[9]='\0'; }
 
     // Country
     const char *co = sv[2].isNull() ? "" : sv[2].as<const char*>();
     strncpy(ac.country, co, 23);  ac.country[23] = '\0';
 
     ac.lat     = lat;
     ac.lon     = lon;
     ac.alt_m   = sv[7].isNull() ? -1.0f : sv[7].as<float>();
     ac.vel_ms  = sv[9].isNull() ? -1.0f : sv[9].as<float>();
     ac.heading = sv[10].isNull() ? 0.0f : sv[10].as<float>();
     ac.color   = acColor(strlen(ac.icao) ? ac.icao : ac.callsign);
     latLonToXY(lat, lon, ac.px, ac.py);
     ac.opx = ac.px;  ac.opy = ac.py;
 
     Serial.printf("  [%d] %-9s lat=%.3f lon=%.3f hdg=%.0f\n",
       radarCount, ac.callsign, lat, lon, ac.heading);
     radarCount++;
   }
   Serial.printf("[Radar] Aircraft tracked: %d\n", radarCount);  Serial.flush();
 
   // Clamp selection index
   if (radarCount > 0) selIdx = constrain(selIdx, 0, radarCount - 1);
   else selIdx = 0;
   prevSelIdx = -1;   // force full redraw of selection after fetch
 }
 
 // ── Sweep line ───────────────────────────────────────────────
static void drawSweepLine(float angle, uint16_t col) {
  for (float a = angle - 0.035f; a <= angle + 0.036f; a += 0.017f) {
    float rad = (a - 90.0f) * DEG_TO_RAD;
    tft.drawLine(RCX, RCY,
      RCX + (int)(cosf(rad) * ROUTER),
      RCY + (int)(sinf(rad) * ROUTER), col);
  }
}
 
 // ── GPS ──────────────────────────────────────────────────────
static void readGPS() {
  // Feed all available GPS bytes into TinyGPSPlus — non-blocking
  while (gpsSerial.available()) gps.encode(gpsSerial.read());

  bool nowFixed = gps.location.isValid() &&
                  gps.location.age() < 3000 &&
                  gps.hdop.isValid() && gps.hdop.hdop() < 5.0f &&
                  gps.satellites.isValid() && gps.satellites.value() >= 4;

  if (nowFixed) {
    radarLat = (float)gps.location.lat();
    radarLon = (float)gps.location.lng();
    gpsSats  = (int)gps.satellites.value();
  }
  bool wasFixed = gpsFixed;
  gpsFixed = nowFixed;

  // Issue 3: on first valid GPS fix, immediately refetch aircraft with correct centre
  if (nowFixed && !wasFixed && !gpsFirstFix) {
    gpsFirstFix = true;
    Serial.println(F("[GPS] First fix — refetching aircraft with GPS coords"));
    Serial.flush();
    fetchAircraft();
    drawBackground();
    drawAllAC();
    if (radarCount > 0) {
      selIdx = constrain(selIdx, 0, radarCount - 1);
      drawSelRing(selIdx, false);
      drawAC(radarAC[selIdx].px, radarAC[selIdx].py,
             radarAC[selIdx].heading, radarAC[selIdx].color, false);
      drawSelTag(selIdx, false);
    }
    lastFetchMs = millis();
  }
}


// ════════════════════════════════════════════════════════════
 //  SETUP
 // ════════════════════════════════════════════════════════════
 void setup() {
   Serial.begin(115200);
   { unsigned long t0 = millis(); while (!Serial && millis()-t0 < 5000) delay(10); }
   delay(500);
   Serial.println(F("\n\n=== FLIGHT RADAR BOOT ==="));  Serial.flush();

  // GPS UART
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println(F("[GPS] Serial1 started on GPIO20/21"));  Serial.flush();
 
   // Encoder
   pinMode(ENC_CLK, INPUT_PULLUP);
   pinMode(ENC_DT,  INPUT_PULLUP);
   pinMode(ENC_SW,  INPUT_PULLUP);
   encLastCLK = digitalRead(ENC_CLK);
   attachInterrupt(digitalPinToInterrupt(ENC_CLK), encoderISR, CHANGE);
 
   // Display
   SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1);
   tft.begin();  tft.setSPISpeed(40000000);  tft.setRotation(2);
   Serial.println(F("[Display] Init OK"));  Serial.flush();
 
  // -- Boot mode select (encoder scroll, SW confirm, 8 s auto-select radar)
  drawModeSelect(bootMode);
  unsigned long modeDeadline = millis() + 8000UL;
  while (millis() < modeDeadline) {
    int d = readEncoder();
    if (d != 0) {
      bootMode = (bootMode + 1) % 2;
      drawModeSelect(bootMode);
      modeDeadline = millis() + 8000UL;
    }
    if (readButton()) break;
    delay(10);
  }
  Serial.printf("[Boot] Mode: %d\n", bootMode);  Serial.flush();

  // GPS-only mode
  if (bootMode == BOOT_MODE_GPS) {
    runGpsStatusScreen();
    return;
  }

  // Flight Radar mode
  tft.fillScreen(COL_BG);
  tft.setTextColor(COL_RING_OUT, COL_BG);  tft.setTextSize(1);
  tft.setCursor(40, 110);  tft.print("Connecting WiFi...");

  Serial.printf("[WiFi] Connecting to %s\n", WIFI_SSID);  Serial.flush();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) { delay(500); Serial.print('.'); }
  Serial.println();  Serial.flush();

  if (WiFi.status() != WL_CONNECTED) {
    tft.fillScreen(COL_BG);  tft.setTextColor(C(255,80,0), COL_BG);
    tft.setCursor(30, 110);  tft.print("WiFi FAILED");
    while (1) delay(1000);
  }
  Serial.printf("[WiFi] Connected: %s\n", WiFi.localIP().toString().c_str());  Serial.flush();

  tft.fillScreen(COL_BG);
  tft.setTextColor(COL_RING_OUT, COL_BG);
  tft.setCursor(50, 110);  tft.print("Authenticating...");
  if (!fetchToken()) {
    tft.fillScreen(COL_BG);  tft.setTextColor(C(255,80,0), COL_BG);
    tft.setCursor(30, 110);  tft.print("Auth FAILED");
    while (1) delay(1000);
  }

  drawBackground();
  fetchAircraft();
  drawBackground();
  drawAllAC();

  // Draw initial selection ring (no red GPS text on boot)
  if (radarCount > 0) {
    drawSelRing(selIdx, false);
    drawAC(radarAC[selIdx].px, radarAC[selIdx].py,
           radarAC[selIdx].heading, radarAC[selIdx].color, false);
    drawSelTag(selIdx, false);
    prevSelIdx = selIdx;
  }

  lastFetchMs = millis();
  lastFrameMs = millis();
  Serial.println(F("[Boot] Radar running"));  Serial.flush();
}

 
 // ════════════════════════════════════════════════════════════
 //  LOOP
 // ════════════════════════════════════════════════════════════
 void loop() {
   unsigned long now = millis();
 
   // ── GPS feed (non-blocking) ─────────────────────────────
  readGPS();
  // ── Read encoder ─────────────────────────────────────────
   int delta = readEncoder();
   if (delta != 0 && radarCount > 0 && !detailOpen) {
     int oldSel = selIdx;
     selIdx = (selIdx + delta + radarCount) % radarCount;
     updateSelection(oldSel, selIdx);
     prevSelIdx = selIdx;
   }
   // While detail panel open, encoder scrolls through aircraft too
   if (delta != 0 && radarCount > 0 && detailOpen) {
     selIdx = (selIdx + delta + radarCount) % radarCount;
     drawDetailPanel();   // just redraws panel contents — no fillScreen
   }
 
   // ── Button ───────────────────────────────────────────────
   if (readButton()) {
     if (!detailOpen) {
       if (radarCount > 0) { detailOpen = true; drawDetailPanel(); }
     } else {
       detailOpen = false;
       closeDetailPanel();
     }
   }
 
   // ── Periodic fetch ────────────────────────────────────────
   if (now - lastFetchMs >= RADAR_FETCH_MS) {
     lastFetchMs = now;
     if (!detailOpen) {
       fetchAircraft();
       drawBackground();
       drawAllAC();
       // Restore selection ring after background repaint
       if (radarCount > 0) {
         drawSelRing(selIdx, false);
         drawAC(radarAC[selIdx].px, radarAC[selIdx].py,
                radarAC[selIdx].heading, radarAC[selIdx].color, false);
         drawSelTag(selIdx, false);
       }
       prevSelIdx = selIdx;
       lastFrameMs = millis();
     }
     return;
   }
 
   // ── Sweep (paused when detail panel open) ────────────────
   if (detailOpen) return;
 
   if (now - lastFrameMs < (unsigned long)FRAME_MS) return;
   lastFrameMs = now;
 
   drawSweepLine(sweepOldAngle, COL_BG);
   drawRings();
   drawCross();
   drawAllAC();
   if (radarCount > 0 && selIdx >= 0 && selIdx < radarCount) {
     drawSelRing(selIdx, false);
     drawAC(radarAC[selIdx].px, radarAC[selIdx].py,
            radarAC[selIdx].heading, radarAC[selIdx].color, false);
     drawSelTag(selIdx, false);
   }
   drawSweepLine(sweepAngle, COL_SWEEP);
   tft.fillCircle(RCX, RCY, 2, COL_SWEEP);

   sweepOldAngle = sweepAngle;
   sweepAngle   += SWEEP_STEP;
  if (sweepAngle >= 360.0f) sweepAngle -= 360.0f;
 }