#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <espnow.h>
#include <ESP8266WiFi.h>

// MPU-6050
Adafruit_MPU6050 mpu;
bool mpu_connected = false;

typedef struct struct_message {
  float gyro_x;
  float gyro_y;
  float gyro_z;
  bool left_click;
  bool right_click;
} struct_message;

struct_message sensorData;

// Receiver MAC Address (replace with your receiver's MAC address)84:0D:8E:A3:A3:30

uint8_t broadcastAddress[] = {0x84, 0x0D, 0x8E, 0xA3, 0xA3, 0x30}; 

const int leftButtonPin = 1; // GPIO0 (pin 3)
const int rightButtonPin = 3; // GPIO2 (pin 1)

void setup() {
  Serial.begin(115200);

  // Initialize I2C with SDA on GPIO0 and SCL on GPIO2
  Wire.begin(2, 0);

  // Attempt to initialize MPU-6050
  if (mpu.begin()) {
    mpu_connected = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
    Serial.println("MPU6050 initialized successfully.");
  } else {
    Serial.println("Failed to find MPU6050 chip, sending dummy data.");
  }

  pinMode(leftButtonPin, INPUT_PULLUP); // Enable internal pull-up resistor
  pinMode(rightButtonPin, INPUT_PULLUP); // Enable internal pull-up resistor

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if (mpu_connected) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sensorData.gyro_x = g.gyro.x;
    sensorData.gyro_y = g.gyro.y;
    sensorData.gyro_z = g.gyro.z;
  } else {
    // Send dummy data if MPU-6050 is not connected
    sensorData.gyro_x = 0.0;
    sensorData.gyro_y = 0.0;
    sensorData.gyro_z = 0.0;
  }
  sensorData.left_click = digitalRead(leftButtonPin) == LOW;
  sensorData.right_click = digitalRead(rightButtonPin) == LOW;
  esp_now_send(broadcastAddress, (uint8_t *)&sensorData, sizeof(sensorData));

  Serial.print("Gyro X: "); Serial.println(sensorData.gyro_x);
  Serial.print("Gyro Y: "); Serial.println(sensorData.gyro_y);
  Serial.print("Gyro Z: "); Serial.println(sensorData.gyro_z);

  delay(10);
}
