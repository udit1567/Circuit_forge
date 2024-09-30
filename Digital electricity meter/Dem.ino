#define BLYNK_TEMPLATE_ID           "TMPL3QHlQnzV3"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "9bGiUYf6eiLUAdmyxZxlYjh-iXzR1E-r"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <PZEM004Tv30.h>//https://github.com/mandulaj/PZEM-004T-v30
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Wire.h>                    // Include Wire library (required for I2C devices)
#include <LiquidCrystal_I2C.h>       // Include LiquidCrystal_I2C library 
#include <ESP8266HTTPClient.h>
PZEM004Tv30 pzem(&Serial);

char auth[] = BLYNK_AUTH_TOKEN;
char ssid [] = "Infinity";
char pass [] = "#Udit1588";
const char* firstEspIP = "192.168.1.20";

float voltage=0;
float current=0;
float power=0;
float energy=0;
float frequency=0;
float pf=0;
unsigned long lastMillis = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);
WiFiClient client;

void setup()
{
 lcd.init();   // initializing the LCD
  lcd.backlight(); // Enable or Turn On the backlight 
 lcd.setCursor(0,0);  //Set cursor to first column of second row
 lcd.print("    Energy meter    ");
 delay(1000);
lcd.setCursor(0,1);  //Set cursor to first column of second row
lcd.print("   Monitor by IOT   ");
delay(1000);
lcd.setCursor(0,2);  //Set cursor to first column of second row
lcd.print("     Designed by   ");
delay(1000);
lcd.setCursor(0,3);  //Set cursor to first column of second row
lcd.print("       UDIT          ");
delay(3000);
lcd.clear();
lcd.print("Device connecting..........."); 
delay(2000);
lcd.setCursor(0,2);  //Set cursor to first column of second row
lcd.print("........please  wait");

Serial.begin(9600);

Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
WiFi.begin(ssid, pass);
lcd.clear();
}
void loop()
{
  
  Blynk.run();



  
    float voltage = pzem.voltage();
    if( !isnan(voltage) ){
        Serial.print("Voltage: "); Serial.print(voltage,1); Serial.println("V");
        lcd.setCursor(0,0); lcd.print("Voltage:      Volts"); lcd.setCursor(8,0); lcd.print(voltage,1);
    } else {
        Serial.println("Error reading voltage");
        lcd.clear();
        lcd.setCursor(0,0); lcd.print("Voltage = 0.00Volts "); 
        lcd.setCursor(0,1); lcd.print("Current = 0 Amps  "); 
        lcd.setCursor(0,3); lcd.print("  No Power  Supply  ");
    }

    float current = pzem.current();
    if( !isnan(current) ){
        Serial.print("Current: "); Serial.print(current,3); Serial.println("A");
        lcd.setCursor(0,1); lcd.print("Current:      Amps"); lcd.setCursor(8,1); lcd.print(current,3);
    } else {
        Serial.println("Error reading current");
        
    }

    float power = pzem.power();
    if( !isnan(power) ){
        Serial.print("Power: "); Serial.print(power,0); Serial.println("W");
        lcd.setCursor(0,2); lcd.print("     Watts"); lcd.setCursor(0,2); lcd.print(power,0);
    } else {
        Serial.println("Error reading power");
            }

    float energy = pzem.energy();
    if( !isnan(energy) ){
        Serial.print("Energy: "); Serial.print(energy, 3); Serial.println("kWh");
        lcd.setCursor(12,2); lcd.print("     Kwh"); lcd.setCursor(12,2); lcd.print(energy/1000,3);
    } else {
        Serial.println("Error reading energy");
    }

    float frequency = pzem.frequency();
    if( !isnan(frequency) ){
        Serial.print("Frequency: "); Serial.print(frequency,1); Serial.println("Hz");
        lcd.setCursor(0,3); lcd.print("Freq:    Hz"); lcd.setCursor(5,3); lcd.print(frequency,1);
    } else {
        Serial.println("Error reading frequency");
    }

    Serial.println();
    delay(200);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(client, "http://" + String(firstEspIP) + "/waterlevel");  // Use client instance here
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        Serial.println("Water level: " + response);
        Blynk.virtualWrite(V7, response);
        lcd.setCursor(12,3); lcd.print("H2O:"); lcd.setCursor(16,3); lcd.print(String(response) + String("%"));
      }
      http.end();
    } 
    
    

delay(1000);

      //Publish data every 5 seconds (5000 milliseconds). Change this value to publish at a different interval.
          if (millis() - lastMillis > 5000) {
            lastMillis = millis();
            
            Blynk.virtualWrite(V1, voltage);
            Blynk.virtualWrite(V2, current);            
            Blynk.virtualWrite(V3, power);
            Blynk.virtualWrite(V4, energy/1000);
            Blynk.virtualWrite(V5, frequency);
            Blynk.virtualWrite(V6, pf);            

          }         
  
}
