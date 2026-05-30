#include <Arduino_Modulino.h>
#include <Arduino_RouterBridge.h>

ModulinoBuzzer buzzer;
ModulinoDistance distanceSensor;
ModulinoMovement movement;
ModulinoThermo thermo;
ModulinoLatchRelay relay;

// ==================== RELAY ====================

int relay_status() {
    return relay.getStatus();
}

void relay_on() {
    relay.set();
}

void relay_off() {
    relay.reset();
}

// ==================== BUZZER ====================

void buzzer_on(int freq) {
    buzzer.tone(freq, 1000);
}

void buzzer_off() {
    buzzer.tone(0, 100);
}

// ==================== DISTANCE ====================

int get_distance() {
    if (distanceSensor.available()) {
        return distanceSensor.get();
    }
    return -1;
}

// ==================== MOVEMENT ====================

String get_movement() {

    movement.update();

    String data = "";

    data += String(movement.getX(), 3) + ",";
    data += String(movement.getY(), 3) + ",";
    data += String(movement.getZ(), 3) + ",";
    data += String(movement.getRoll(), 1) + ",";
    data += String(movement.getPitch(), 1) + ",";
    data += String(movement.getYaw(), 1);

    return data;
}

// ==================== THERMO ====================

String get_thermo() {

    float temperature = thermo.getTemperature();
    float humidity = thermo.getHumidity();

    String data = "";
    data += String(temperature, 2);
    data += ",";
    data += String(humidity, 2);

    return data;
}

float get_temperature() {
    return thermo.getTemperature();
}

float get_humidity() {
    return thermo.getHumidity();
}

// ==================== SETUP ====================

void setup() {

    Bridge.begin();

    Modulino.begin();

    relay.begin();
    buzzer.begin();
    distanceSensor.begin();
    movement.begin();
    thermo.begin();

    // Relay
    Bridge.provide("relay_on", relay_on);
    Bridge.provide("relay_off", relay_off);
    Bridge.provide("relay_status", relay_status);

    // Buzzer
    Bridge.provide("buzzer_on", buzzer_on);
    Bridge.provide("buzzer_off", buzzer_off);

    // Distance
    Bridge.provide("get_distance", get_distance);

    // Movement
    Bridge.provide("get_movement", get_movement);

    // Thermo
    Bridge.provide("get_thermo", get_thermo);
    Bridge.provide("get_temperature", get_temperature);
    Bridge.provide("get_humidity", get_humidity);
}

void loop() {}