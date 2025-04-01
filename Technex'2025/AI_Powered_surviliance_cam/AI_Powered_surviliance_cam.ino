#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>
#include "esp_camera.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "Excitel 2.4";
const char* password = "#Udit1588";

// Server endpoint
const String serverURL = "http://192.168.1.46:5000/detect_objects_base64";
const String authToken = "CeI1OU68"; // Replace with your actual token

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Camera pin definitions
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define BUTTON_PIN 13
#define BUZZER_PIN 2
#define FLASH_PIN 12

bool flashState = false;

void displayResults(const String& text) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0); // Start text from the beginning
    display.println(text);
    display.display();
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    displayResults("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    String ipAddress = WiFi.localIP().toString();
    Serial.println("Connected! IP: " + ipAddress);
    
    pinMode(4, OUTPUT);  // Set GPIO 4 as output for flash
    digitalWrite(4, LOW); // Ensure flash is off initially

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(FLASH_PIN, INPUT_PULLUP);

    Wire.begin(15, 14);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
        while (1);
    }

    // Camera configuration
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
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 20;
    config.fb_count = 2;

    if (esp_camera_init(&config) != ESP_OK) {
        displayResults("Camera Error");
        while (1);
    }

    displayResults("Smart Hive\nIP: " + ipAddress + "\nReady!\nPress Button");
}

void captureAndSendImage() {
    displayResults("Capturing...");

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        displayResults("Capture Failed");
        return;
    }

    String base64Image = base64::encode(fb->buf, fb->len);
    esp_camera_fb_return(fb);

    if (base64Image.isEmpty()) {
        displayResults("Encoding Error");
        return;
    }

    DynamicJsonDocument doc(3072);
    doc["image_base64"] = base64Image;
    String payload;
    serializeJson(doc, payload);

    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", authToken);

    int httpCode = http.POST(payload);
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();

        DynamicJsonDocument responseDoc(1024);
        DeserializationError error = deserializeJson(responseDoc, response);

        if (error) {
            displayResults("JSON Error");
            return;
        }

        JsonObject counts = responseDoc["counts"];
        int total = responseDoc["total_objects"];

        String displayText = "";
        for (JsonPair kv : counts) {
            displayText += String(kv.key().c_str()) + ": " + kv.value().as<String>() + "\n";
        }
        displayText += "Total: " + String(total);

        displayResults(displayText);

        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
    } else {
        displayResults("Error: " + String(httpCode));
    }
    http.end();
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        captureAndSendImage();
        delay(2000);
    }

    if (digitalRead(FLASH_PIN) == LOW) {
        flashState = !flashState;
        digitalWrite(4, flashState ? HIGH : LOW); // GPIO 4 controls flash
        displayResults(flashState ? "Flash: ON" : "Flash: OFF");
        delay(500);
    }
}