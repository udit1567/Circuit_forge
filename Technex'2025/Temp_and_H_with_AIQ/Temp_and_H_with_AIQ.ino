#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Wi-Fi credentials
const char *ssid = "Wifi";
const char *password = "#Udit1588";

// API details
const String apiToken = "CeI1OU68";
const String apiUrl = "http://192.168.246.24:5000/update"; // Flask API endpoint

// Sensor Pins
#define MQ135_PIN 35  // Analog pin for MQ135
#define DHTPIN 21     // GPIO pin for DHT11
#define DHTTYPE DHT11 // DHT sensor type

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Constants (Adjust after calibration)
#define RZERO 76.63
#define PARA 116.6020682
#define PARB 2.769034857

WiFiClient wifiClient;
unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // Send data every 10 seconds

// Function to calculate resistance of the sensor
float getResistance(int adcValue) {
  float voltage = adcValue * (3.3 / 4095.0); // Convert ADC to Voltage
  float resistance = ((3.3 * 10.0) / voltage) - 10.0; // Using a 10kΩ Load Resistor
  return resistance;
}

// Function to get CO2 PPM from MQ135
float getPPM(int adcValue) {
  float resistance = getResistance(adcValue);
  float ratio = resistance / RZERO;
  float ppm = PARA * pow(ratio, -PARB); // Calculate CO2 PPM
  return ppm;
}

// Function to estimate AQI from CO2 PPM
int calculateAQI(float ppm) {
  if (ppm <= 50) return 50;
  else if (ppm <= 100) return 100;
  else if (ppm <= 150) return 150;
  else if (ppm <= 200) return 200;
  else if (ppm <= 250) return 250;
  else if (ppm <= 300) return 300;
  else return 500; // Max AQI for high pollution
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 Sensor Node...");
  
  dht.begin();
  connectWiFi();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi disconnected! Reconnecting...");
      connectWiFi();
    }

    // Read temperature and humidity
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Read MQ135 Sensor
    int sensorValue = analogRead(MQ135_PIN);
    float ppm = getPPM(sensorValue);
    int aqi = calculateAQI(ppm);

    Serial.printf("Temp: %.2f°C | Humidity: %.2f%% | CO2 PPM: %.2f | AQI: %d\n",
                  temperature, humidity, ppm, aqi);

    // Create JSON payload
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["D1"] = temperature;
    jsonDoc["D2"] = humidity;
    jsonDoc["D4"] = aqi;
    jsonDoc["D5"] = ppm;

    String jsonPayload;
    serializeJson(jsonDoc, jsonPayload);

    // Send HTTP POST request
    HTTPClient http;
    http.begin(wifiClient, apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", apiToken);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.printf("Error sending POST request: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    lastTime = millis();
  }
}
