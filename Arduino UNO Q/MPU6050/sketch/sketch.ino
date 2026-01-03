// SPDX-FileCopyrightText: Copyright (C) ARDUINO SRL
//
// SPDX-License-Identifier: MPL-2.0

#include <Wire.h>
#include <Arduino_RouterBridge.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Acceleration values (m/s^2)
float x_accel, y_accel, z_accel;

unsigned long previousMillis = 0;
const unsigned long interval = 16; // ~62.5 Hz

void setup() {
  Bridge.begin();
  Monitor.begin();

  Monitor.println("=== MPU6050 ACCEL BOOT ===");

  Wire.begin();

  if (!mpu.begin()) {
    Monitor.println("[ERROR] MPU6050 not found");
    while (1) {
      delay(1000);
    }
  }

  // Accelerometer configuration
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Monitor.println("[OK] MPU6050 initialized");
  Monitor.println("Accel units: m/s^2");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    x_accel = accel.acceleration.x;
    y_accel = accel.acceleration.y;
    z_accel = accel.acceleration.z;

    // Local debug
    Monitor.print("[ACC] ");
    Monitor.print(x_accel, 4); Monitor.print(", ");
    Monitor.print(y_accel, 4); Monitor.print(", ");
    Monitor.println(z_accel, 4);

    // 🔗 Send to Linux (same receiver)
    Bridge.notify(
      "record_sensor_movement",
      x_accel,
      y_accel,
      z_accel
    );
  }
}
