/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#include <Arduino_RPClite.h>

SerialTransport transport(Serial1);
RPCServer server(transport);

int add(int a, int b){
    return a+b;
}

int add2(int a, int b){
    return a+b;
}

String greet(){
    return String("Hello Friend");
}

String loopback(String message){
    return message;
}

class multiplier {
public:

    multiplier(){}
    static int mult(int a, int b){
        return a*b;
    }
};


float multip(float a, float b) {
    return a*b;
}

void setup() {
    Serial1.begin(115200);
    while(!Serial1);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);
    while(!Serial);

    server.bind("add", add);

    if (!server.bind("add", add2)){
        Serial.println("server refused to bind same name twice");
    }

    server.bind("greet", greet);
    server.bind("loopback", loopback);
    server.bind("another_greeting", [] {return MsgPack::str_t ("This is a lambda greeting");});
    server.bind("object_multi", &multiplier::mult);
    server.bind("multip", multip);

}

void blink_before(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
}

void loop() {
    blink_before();
    server.run();
}