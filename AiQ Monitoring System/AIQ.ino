#include <WiFi.h>
#include <HTTPClient.h>
#include <MQUnifiedsensor.h>
#include <DHT.h>
#define DHTPIN 4    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
const char * ssid = "Infinity";
const char * password = "#Udit1588";
#define Board "ESP-32"
#define Voltage_Resolution 5
#define pin 36 //Analog input 0 of your arduino
#define Pin1 34
#define type "MQ-135" //MQ135
#define Type1 "MQ-2" //MQ135
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
#define RatioMQ2CleanAir 9.83//RS / R0 = 3.6 ppm 
DHT dht(DHTPIN, DHTTYPE);

MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin1, Type1);

void setup() {
  Serial.begin(9600); //Init serial port
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi");
  Serial.println("Started");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  dht.begin();
  Serial.println("\nConnected to Wi-Fi");
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setRegressionMethod(1);
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ135.init(); 
  MQ2.init(); 
  Serial.print("Calibrating please wait.");
  delay(200);
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  MQ2.setR0(calcR0/10);
  Serial.println("  done!.");
  delay(200);
  MQ2.serialDebug(true);
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  Serial.println("** Values from MQ-135 ****");
  delay(200);
  Serial.println("|    CO   |  Alcohol |   CO2  |  Toluen  |  NH4  |  Aceton  |");  
}

void loop() {
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
  float CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(77.255); MQ135.setB(-3.18); //Configure the equation to calculate Alcohol concentration value
  float Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47); MQ135.setB(-2.862); // Configure the equation to calculate CO2 concentration value
  float CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(44.947); MQ135.setB(-3.445); // Configure the equation to calculate Toluen concentration value
  float Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure the equation to calculate NH4 concentration value
  float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(34.668); MQ135.setB(-3.369); // Configure the equation to calculate Aceton concentration value
  float Aceton = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  MQ2.update();
  MQ2.setA(658.71); MQ2.setB(-2.168);
  float Propane = MQ2.readSensor();
  MQ2.setA(987.99); MQ2.setB(-2.162);
  float H2 = MQ2.readSensor();
  delay(1000);
  Serial.print("|   "); Serial.print(CO); 
  Serial.print("   |   "); Serial.print(Alcohol);
  Serial.print("   |   "); Serial.print(CO2 + 400); 
  Serial.print("   |   "); Serial.print(Toluen); 
  Serial.print("   |   "); Serial.print(NH4); 
  Serial.print("   |   "); Serial.print(Aceton);
  Serial.print("   |   "); Serial.print(Propane); 
  Serial.print("   |   "); Serial.print(H2);
  Serial.println("   |");
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }  
  makeHTTPRequest(t, h, CO, NH4, CO2, Toluen, Aceton, Propane, H2);
  delay(2000);
}
void makeHTTPRequest(float temperature, float humidity, float CO, float NH4, float CO2, float Toluen, float Aceton,float Propane,float H2) {
  // Make HTTP GET request to the provided URL with provided values
  HTTPClient http;
//https://script.google.com/macros/s/AKfycbwQQelufDeFnUnTefwMyKXsEqGKqu9mup8mGSdJ_Ffu0UCZ67OOp62Msp1o-lnKiHlLPQ/exec
  String url = "https://script.google.com/macros/s/AKfycbwQQelufDeFnUnTefwMyKXsEqGKqu9mup8mGSdJ_Ffu0UCZ67OOp62Msp1o-lnKiHlLPQ/exec";
  url += "?temperature=" + String(temperature) +
         "&humidity=" + String(humidity) +
         "&CO=" + String(CO) +
         "&NH4=" + String(NH4) +
         "&CO2=" + String(CO2+400) +
         "&Toluen=" + String(Toluen) +
         "&Aceton=" + String(Aceton) +
         "&Propane=" + String(Propane) +
         "&H2=" + String(H2);
  Serial.println(Propane);
  Serial.println("Making HTTP request to: " + url);

  if (http.begin(url)) {  // HTTP connection
    int httpCode = http.GET();  // Start the request

    if (httpCode > 0) {  // Check for a valid HTTP status code
      String payload = http.getString(); // Get the request response payload
      Serial.println("HTTP Response code: " + String(httpCode));
      Serial.println("Response payload: " + payload);
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end(); // Free resources
  } else {
    Serial.println("Unable to connect to server");
  }
}
