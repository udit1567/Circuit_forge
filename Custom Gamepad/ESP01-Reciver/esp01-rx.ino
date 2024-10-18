#include <espnow.h>
#include <ESP8266WiFi.h>

// Define the structure to hold received data
typedef struct struct_message {
  int joyX;
  int joyY;
  bool button1;
  bool button2;
  bool button3;
  bool button4;
  float acc_x;
  float acc_y;
} struct_message;

struct_message sensorData;

// Callback function that gets called when data is received
void onDataReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&sensorData, incomingData, sizeof(sensorData));

  // Print received data in a single line with commas
  Serial.print(sensorData.joyX);
  Serial.print(", ");
  Serial.print(sensorData.joyY);
  Serial.print(", ");
  Serial.print(sensorData.button1);
  Serial.print(", ");
  Serial.print(sensorData.button2);
  Serial.print(", ");
  Serial.print(sensorData.button3);
  Serial.print(", ");
  Serial.print(sensorData.button4);
  Serial.print(", ");
  Serial.print(sensorData.acc_x);
  Serial.print(", ");
  Serial.print(sensorData.acc_y);
  Serial.println(); // End the line
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP-01 Receiver Started");

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the callback function for receiving data
  esp_now_register_recv_cb(onDataReceive);
}

void loop() {
  // Nothing to do here, all handling is done in the callback
  delay(10); // Adjust delay as needed
}
