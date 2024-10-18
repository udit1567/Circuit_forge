#include <espnow.h>
#include <ESP8266WiFi.h>

// Define the structure for the data to be transmitted
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

// Receiver MAC Address (replace with your receiver's MAC address)
uint8_t broadcastAddress[] = {0x84, 0x0D, 0x8E, 0xA3, 0xA3, 0x30};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP8266");

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
  // Check if data is available from UART (from Arduino)
  if (Serial.available()) {
    String dataReceived = Serial.readStringUntil('\n'); // Read until newline
    int dataIndex = 0;
    char* data = new char[dataReceived.length() + 1];
    dataReceived.toCharArray(data, dataReceived.length() + 1);
    
    // Parse the received data
    char* token = strtok(data, ",");
    while (token != NULL) {
      switch (dataIndex) {
        case 0: sensorData.joyX = atoi(token); break; // Joystick X
        case 1: sensorData.joyY = atoi(token); break; // Joystick Y
        case 2: sensorData.button1 = (atoi(token) == LOW); break; // Button 1
        case 3: sensorData.button2 = (atoi(token) == LOW); break; // Button 2
        case 4: sensorData.button3 = (atoi(token) == LOW); break; // Button 3
        case 5: sensorData.button4 = (atoi(token) == LOW); break; // Button 4
        case 6: sensorData.acc_x = atof(token); break; // Acceleration X
        case 7: sensorData.acc_y = atof(token); break; // Acceleration Y
      }
      token = strtok(NULL, ",");
      dataIndex++;
    }
    
    delete[] data; // Free the allocated memory

    // Send the data via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *)&sensorData, sizeof(sensorData));
  }

  delay(10); // Adjust as needed
}
