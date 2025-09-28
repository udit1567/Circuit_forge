/******************************************************************************
 * ESP32-CAM Video Streamer with Remote & Physical Controls + OLED Display
 *
 * This sketch builds upon the basic streamer to add an I2C OLED display
 * that shows the name of an object detected by a remote client, and allows
 * for physical control of the flash.
 *
 * It adds a new endpoint:
 * 6. http://<ESP_IP>/update_oled (POST request with object name in body)
 *
 * Instructions:
 * 1. Install Libraries: In Arduino IDE, go to Library Manager and install:
 * - Adafruit GFX Library
 * - Adafruit SSD1306
 * 2. Wire the OLED Display:
 * - OLED GND -> ESP32-CAM GND
 * - OLED VCC -> ESP32-CAM 3.3V
 * - OLED SDA -> ESP32-CAM GPIO 15
 * - OLED SCL -> ESP32-CAM GPIO 14
 * 3. Wire the Button:
 * - Connect a momentary push button between GPIO 12 and GND.
 * 4. Upload this sketch to your ESP32-CAM.
 ******************************************************************************/

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Disable brownout detector
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

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
#define FLASH_GPIO_NUM     4

// --- PIN DEFINITION FOR OLED (I2C) ---
#define OLED_SDA_PIN      15
#define OLED_SCL_PIN      14
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- PIN DEFINITION FOR BUTTON ---
#define BUTTON_GPIO_NUM   12

WebServer server(80);

// --- Global State Variables ---
bool flashState = false;
String detectedObjects = "None"; // To store object names from Python
// Button debounce variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = HIGH;

// Forward declarations
void updateOledStatus();
void setFlashState(bool newState);
void handleButton();
void handleOledUpdate();

// --- Web Server Handlers ---

// Handler for the root URL
void handleRoot() {
  server.send(200, "text/plain", "ESP32-CAM Stream Ready! Navigate to /stream");
}

// Handler to receive object data from Python client (via POST request)
void handleOledUpdate(){
  if (server.hasArg("plain")== false){
    server.send(200, "text/plain", "Body not received");
    return;
  }
  String newObjects = server.arg("plain");
  if (newObjects != detectedObjects) {
    detectedObjects = newObjects;
    updateOledStatus(); // Update display immediately
  }
  server.send(200, "text/plain", "OK");
}

// Handler for the video stream (no changes here)
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
    if (client.connected()) {
      client.print("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(fb->len) + "\r\n\r\n");
      client.write(fb->buf, fb->len);
      client.print("\r\n");
    }
    esp_camera_fb_return(fb);
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
  } else if (var == "flash") {
    setFlashState(val == 1);
  } else {
    res = -1;
  }

  if (res == 0) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", "Command failed");
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // --- Initialize OLED Display ---
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Initializing...");
  display.display();

  // --- Setup Button Pin ---
  pinMode(BUTTON_GPIO_NUM, INPUT_PULLUP);

  // --- Initialize Camera ---
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
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    display.clearDisplay(); display.setCursor(0,0); display.println("Camera Init Failed!"); display.display();
    return;
  }
  
  // Setup flash LED
  ledcSetup(LEDC_CHANNEL_0, 5000, 8);
  ledcAttachPin(FLASH_GPIO_NUM, LEDC_CHANNEL_0);
  setFlashState(false); // Start with flash off and update display

  // --- WiFi Connection ---
  display.clearDisplay(); display.setCursor(0,0); display.println("Connecting to WiFi..."); display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  Serial.print("Stream: http://"); Serial.println(WiFi.localIP());
  
  updateOledStatus(); // Show initial status on OLED

  // --- Start Server ---
  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.on("/control", handleControl);
  server.on("/update_oled", HTTP_POST, handleOledUpdate); // New endpoint for OLED
  
  server.begin();
}

void loop() {
  server.handleClient();
  handleButton();
}

// --- Helper Functions ---

// Updates the OLED with the current status
void updateOledStatus() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(WiFi.localIP().toString());
  display.print("Flash: ");
  display.println(flashState ? "ON" : "OFF");
  display.drawFastHLine(0, 20, 128, WHITE);
  display.setCursor(0, 25);
  display.setTextSize(2);
  display.println("Object:");
  display.setTextSize(1);
  display.setTextWrap(true); // Enable text wrapping for long object lists
  display.setCursor(0, 45);
  display.println(detectedObjects);
  display.display();
  display.setTextWrap(false); // Disable for predictable layout elsewhere
}

// Central function to control the flash state
void setFlashState(bool newState) {
  flashState = newState;
  ledcWrite(LEDC_CHANNEL_0, flashState ? 255 : 0);
  updateOledStatus(); // Update display whenever flash state changes
}

// Handles reading the physical button with debouncing
void handleButton() {
  int reading = digitalRead(BUTTON_GPIO_NUM);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Check for a falling edge (button press)
    if (reading == LOW && lastButtonState == HIGH) {
      setFlashState(!flashState); // Toggle the flash
    }
  }
  
  lastButtonState = reading;
}

