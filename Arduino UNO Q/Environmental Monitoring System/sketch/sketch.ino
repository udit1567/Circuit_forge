// SPDX-License-Identifier: MPL-2.0

#include <Arduino.h>
#include <Wire.h>
#include <Arduino_RouterBridge.h>

#include <Adafruit_BMP280.h>
#include <MQ135_XCR.h>
#include <MQGasKit.h>

/* ================= SENSORS ================= */
Adafruit_BMP280 bmp;
MQ135_XCR mq135(A0);
MQGasKit  mq2(A1, MQ2);

/* ================= FILTERING ================= */
const float alpha = 0.85;

float mq135_co2_f = 0;
float mq2_lpg_f   = 0;
float mq2_co_f    = 0;
float mq2_smoke_f = 0;

/* ================= TIMING ================= */
unsigned long lastMillis = 0;
const unsigned long interval = 2000;

void setup() {
  Bridge.begin();
  Wire.begin();

  bmp.begin(0x76);

  mq135.calibrate();
  mq2.calibrate();
}

void loop() {
  unsigned long now = millis();
  if (now - lastMillis >= interval) {
    lastMillis = now;

    /* -------- BMP280 -------- */
    float temperature = bmp.readTemperature();
    float pressure    = bmp.readPressure() / 100.0;
    float altitude    = bmp.readAltitude(1013.25);

    /* -------- MQ135 -------- */
    float mq135_raw = mq135.getPPM() + 400;
    String airQuality = mq135.getAirQuality();
    mq135_co2_f = alpha * mq135_raw + (1 - alpha) * mq135_co2_f;

    /* -------- MQ2 -------- */
    float lpg_raw   = mq2.getPPM("LPG");
    float co_raw    = mq2.getPPM("CO");
    float smoke_raw = mq2.getPPM("Smoke");

    mq2_lpg_f   = alpha * lpg_raw   + (1 - alpha) * mq2_lpg_f;
    mq2_co_f    = alpha * co_raw    + (1 - alpha) * mq2_co_f;
    mq2_smoke_f = alpha * smoke_raw + (1 - alpha) * mq2_smoke_f;

    /* -------- SEND TO LINUX -------- */
    Bridge.notify("bmp_temperature", temperature);
    Bridge.notify("bmp_pressure", pressure);
    Bridge.notify("bmp_altitude", altitude);

    Bridge.notify("mq135_co2", mq135_co2_f);
    Bridge.notify("mq135_aq", airQuality);

    Bridge.notify("mq2_lpg", mq2_lpg_f);
    Bridge.notify("mq2_co", mq2_co_f);
    Bridge.notify("mq2_smoke", mq2_smoke_f);
  }
}
