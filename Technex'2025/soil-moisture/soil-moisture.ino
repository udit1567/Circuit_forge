#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char *ssid = "Wifi";
const char *password = "#Udit1588";

// API details
const String apiToken = "CeI1OU68";
const String apiUrl = "http://192.168.246.24:5000/update"; // Flask server URL

WiFiClient wifiClient;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // 10 seconds

void setup() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void loop() {
    if ((millis() - lastTime) > timerDelay) {
        if (WiFi.status() == WL_CONNECTED) {
            int value = analogRead(A0);
            int soil = map(value,540,845,100,0);

            StaticJsonDocument<100> jsonDoc;
            jsonDoc["D3"] = soil;
            String jsonPayload;
            serializeJson(jsonDoc, jsonPayload);

            HTTPClient http;
            http.begin(wifiClient, apiUrl);
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Authorization", apiToken);
            http.POST(jsonPayload);
            http.end();
        }
        lastTime = millis();
    }
}
