#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

const int joystickX = A0;
const int joystickY = A1;
const int button1 = 2;
const int button2 = 3;
const int button3 = 4;
const int button4 = 5;

void setup() {
  Serial.begin(115200);  // UART communication with ESP-01
  Serial.println("Starting");

  // Setup buttons as inputs
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  } else {
    Serial.println("MPU6050 connection successful");
  }

  // Set MPU6050 ranges (optional)
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  // Read joystick values
  int joyX = analogRead(joystickX);
  int joyY = analogRead(joystickY);

  // Read button states
  int buttonState1 = digitalRead(button1);
  int buttonState2 = digitalRead(button2);
  int buttonState3 = digitalRead(button3);
  int buttonState4 = digitalRead(button4);

  // Read MPU6050 sensor event
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Send data to ESP-01 via UART (Serial)
  String dataToSend = String(joyX) + "," + String(joyY) + "," +
                      String(buttonState1) + "," + String(buttonState2) + "," +
                      String(buttonState3) + "," + String(buttonState4) + "," +
                      String(g.gyro.y) + "," + String(g.gyro.z);

  Serial.println(dataToSend);  // Send data as a comma-separated string

  delay(10);  // Adjust delay as needed
}
