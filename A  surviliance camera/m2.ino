/******************************************************************************
 * ESP32-CAM Video Streamer with Remote Controls
 * * This sketch configures the ESP32-CAM to:
 * 1. Connect to a specified Wi-Fi network.
 * 2. Initialize the camera module (AI-THINKER model).
 * 3. Start a web server on port 80.
 * 4. Provide a video stream at http://<ESP_IP>/stream
 * 5. Provide a control endpoint at http://<ESP_IP>/control
 * - ?var=framesize&val=<0-10> -> Changes video resolution
 * - ?var=flash&val=<0-1> -> Toggles the on-board LED flash
 * * Instructions:
 * 1. Open this file in the Arduino IDE.
 * 2. Go to File > Preferences and add the ESP32 boards URL:
 * https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 * 3. Go to Tools > Board > Boards Manager, search for "esp32" and install it.
 * 4. Select the correct board: "AI Thinker ESP32-CAM".
 * 5. Fill in your WiFi credentials in the ssid and password variables below.
 * 6. Upload the code to your ESP32-CAM.
 * 7. Open the Serial Monitor at 115200 baud to see the IP address.
 ******************************************************************************/

#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <WebServer.h>

// IMPORTANT: Replace with your network credentials
const char* ssid = "Excitel_ 2.4";
const char* password = "@Udit1588";

// --- PIN DEFINITION FOR AI-THINKER ESP32-CAM ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_GPIO_NUM     4 // On-board LED

WebServer server(80);

void startCameraServer();
void setupLedFlash(int pin);

// Handler for the root URL
void handleRoot() {
  server.send(200, "text/plain", "ESP32-CAM Stream Ready! Navigate to /stream");
}

// Handler for the video stream
void handleStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (true) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    if (client.connected()) {
      client.print(response);
      client.write(fb->buf, fb->len);
      client.print("\r\n");
    }

    esp_camera_fb_return(fb);

    // Allow the server to handle other clients (like control commands)
    // This is a workaround for the blocking nature of the stream loop
    server.handleClient();

    if (!client.connected()) {
      break;
    }
  }
}

// Handler for control commands
void handleControl() {
  String var = server.arg("var");
  String val_str = server.arg("val");
  int val = val_str.toInt();

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;

  if (var == "framesize") {
    if (s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
  }
  else if (var == "flash") {
    ledcWrite(0, val ? 255: 0); // Use LEDC to control brightness
  }
  else {
    res = -1; // Unknown variable
  }

  if (res == 0) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", "Command failed");
  }
}

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Start with a medium resolution
  config.frame_size = FRAMESIZE_VGA; // 640x480
  config.jpeg_quality = 12; // 0-63 lower means higher quality
  config.fb_count = 2;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // Setup flash LED
  setupLedFlash(FLASH_GPIO_NUM);

  // WiFi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print IP address
  Serial.print("Camera Stream: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");

  // Start server
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.on("/control", handleControl);
  
  server.begin();
}

void loop() {
  server.handleClient();
}

void setupLedFlash(int pin) {
  ledcSetup(0, 5000, 8); // channel 0, 5000 Hz, 8-bit resolution
  ledcAttachPin(pin, 0); // attach pin to channel 0
}

