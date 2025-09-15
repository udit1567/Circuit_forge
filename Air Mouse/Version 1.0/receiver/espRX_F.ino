#include <espnow.h>
#include <ESP8266WiFi.h>

typedef struct struct_message {
  float gyro_x;
  float gyro_y;
  float gyro_z;
  bool left_click;
  bool right_click;
} struct_message;

struct_message receivedData;

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print(receivedData.gyro_x);
  Serial.print(",");
  Serial.print(receivedData.gyro_y);
  Serial.print(",");
  Serial.print(receivedData.gyro_z);
  Serial.print(",");
  Serial.print(receivedData.left_click);
  Serial.print(",");
  Serial.println(receivedData.right_click);
}

void setup() {
  Serial.begin(115200);

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  // Loop is empty, data is handled in the callback
}