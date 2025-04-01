#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Infinity";
const char* password = "#Udit1588";

WebServer server(80);
const int solenoidPin = 25;

void handleUnlock() {
    digitalWrite(solenoidPin, HIGH);
    delay(5000);
    digitalWrite(solenoidPin, LOW);
    server.send(200, "text/plain", "Door unlocked");
}

void setup() {
    Serial.begin(115200);
    pinMode(solenoidPin, OUTPUT);
    digitalWrite(solenoidPin, LOW);
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.println(WiFi.localIP());
    
    server.on("/unlock", HTTP_GET, handleUnlock);
    server.begin();
    Serial.println("Server started");
}

void loop() {
    server.handleClient();
}
