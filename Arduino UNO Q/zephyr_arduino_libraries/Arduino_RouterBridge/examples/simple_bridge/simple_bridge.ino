/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#include <Arduino_RouterBridge.h>


bool set_led(bool state) {
    digitalWrite(LED_BUILTIN, state);
    return state;
}

int add(int a, int b) {
    return a + b;
}

String greet() {
    return String("Hello Friend");
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    pinMode(LED_BUILTIN, OUTPUT);

    Bridge.begin();

    if (!Bridge.provide("set_led", set_led)) {
        Serial.println("Error providing method: set_led");
    } else {
        Serial.println("Registered method: set_led");
    }

    Bridge.provide("add", add);

    Bridge.provide_safe("greet", greet);

}

void loop() {
    float res;
    if (!Bridge.call("multiply", 1.0, 2.0).result(res)) {
        Serial.println("Error calling method: multiply");
    };

    // Call with deferred response check
    RpcResult outcome = Bridge.call("multiply", 5.0, 7.0);
    Serial.println("RPC called");
    delay(10);
    if (outcome.result(res)) {
        Serial.print("Result of the operation is: ");
        Serial.println(res);
    } else {
        Serial.println(outcome.error.code);
        Serial.println(outcome.error.traceback);
    }

    Bridge.notify("signal", 200);
}
