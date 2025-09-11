#include <BleMouse.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

BleMouse bleMouse("ESP32 BLE Mouse");
Adafruit_MPU6050 mpu;

// Button pins
#define LEFT_CLICK_PIN  12
#define RIGHT_CLICK_PIN 13
#define SCROLL_UP_PIN   14
#define SCROLL_DOWN_PIN 27

// Sensitivity and deadzone
float sensitivity = 15.0;       // Adjust this to change speed
float thresholdX = 0.1;         // Deadzone for X
float thresholdY = 0.15;        // Slightly higher to avoid upward drift

// Gyro calibration offsets
float gyroX_offset = 0;
float gyroY_offset = 0;
float gyroZ_offset = 0;

// --- Moving Average Filter ---
#define FILTER_SIZE 5
float xBuffer[FILTER_SIZE] = {0};
float yBuffer[FILTER_SIZE] = {0};
int filterIndex = 0;

float smooth(float *buffer, float newVal) {
  buffer[filterIndex] = newVal;
  float sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) sum += buffer[i];
  return sum / FILTER_SIZE;
}

// --- Gyro Calibration ---
void calibrateGyro(int samples = 500) {
  Serial.println("Calibrating gyroscope... keep device still");

  float sumX = 0, sumY = 0, sumZ = 0;
  float gx[samples], gy[samples], gz[samples];

  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    gx[i] = g.gyro.x;
    gy[i] = g.gyro.y;
    gz[i] = g.gyro.z;

    sumX += gx[i];
    sumY += gy[i];
    sumZ += gz[i];

    delay(4);
  }

  gyroX_offset = sumX / samples;
  gyroY_offset = sumY / samples;
  gyroZ_offset = sumZ / samples;

  Serial.println("Calibration complete!");
  Serial.printf("Offsets -> X: %.6f, Y: %.6f, Z: %.6f\n", gyroX_offset, gyroY_offset, gyroZ_offset);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Mouse with MPU6050 (RAW)");

  bleMouse.begin();

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found, check wiring!");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_10_HZ);

  pinMode(LEFT_CLICK_PIN, INPUT_PULLUP);
  pinMode(RIGHT_CLICK_PIN, INPUT_PULLUP);
  pinMode(SCROLL_UP_PIN, INPUT_PULLUP);
  pinMode(SCROLL_DOWN_PIN, INPUT_PULLUP);

  calibrateGyro();

  Serial.println("Setup complete. Waiting for BLE connection...");
}

void loop() {
  if (bleMouse.isConnected()) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Remove gyro bias
    float gyroX = g.gyro.x - gyroX_offset; // Roll
    float gyroY = g.gyro.y - gyroY_offset; // Pitch
    float gyroZ = g.gyro.z - gyroZ_offset; // Yaw

    // Map gyro axes to cursor movement
    float moveX = gyroZ; // Horizontal movement
    float moveY = gyroY; // Vertical movement

    // Apply deadzone
    if (fabs(moveX) < thresholdX) moveX = 0;
    if (fabs(moveY) < thresholdY) moveY = 0;

    // Smooth output
    filterIndex = (filterIndex + 1) % FILTER_SIZE;
    moveX = smooth(xBuffer, moveX);
    moveY = smooth(yBuffer, moveY);

    // Clamp residuals to avoid drift
    if (fabs(moveX) < 0.05) moveX = 0;
    if (fabs(moveY) < 0.05) moveY = 0;

    // Scale linearly using sensitivity (no dynamic scaling)
    moveX *= sensitivity;
    moveY *= sensitivity;

    // Send mouse movement
    if ((int)moveX != 0 || (int)moveY != 0) {
      bleMouse.move((int)-moveX, (int)moveY); // Invert X for natural feel
    }

    // --- Button Controls ---
    if (digitalRead(LEFT_CLICK_PIN) == LOW) {
      bleMouse.press(MOUSE_LEFT);
    } else {
      bleMouse.release(MOUSE_LEFT);
    }

    if (digitalRead(RIGHT_CLICK_PIN) == LOW) {
      bleMouse.press(MOUSE_RIGHT);
    } else {
      bleMouse.release(MOUSE_RIGHT);
    }

    if (digitalRead(SCROLL_UP_PIN) == LOW) {
      bleMouse.move(0, 0, 1); // scroll up
      delay(150);
    }

    if (digitalRead(SCROLL_DOWN_PIN) == LOW) {
      bleMouse.move(0, 0, -1); // scroll down
      delay(150);
    }
  }

  delay(5); // control loop delay
}
