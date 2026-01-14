/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#include <Arduino_RPClite.h>

SerialTransport transport(Serial1);
RPCClient client(transport);

void setup() {
    Serial1.begin(115200);
    while(!Serial1);

    pinMode(LED_BUILTIN, OUTPUT);
    delay(10);

    Serial.begin(115200);
    while(!Serial);
}

void blink_before(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
}

void loop() {
    float result;
    blink_before();

    String str_res;
    bool ok = client.call("loopback", str_res, "Sending a greeting");
    Serial.println(str_res);

    ok = client.call("mult", result, 2.0, 3.0);

    if (ok) {
        Serial.print("Result: ");
        Serial.println(result);
    }

    ok = client.call("divi", result, 2.0, 0.0);
    if (!ok) {
        Serial.print("Testing Server-side exception OK. ERR code: ");
        Serial.print(client.lastError.code);
        Serial.print(" ERR trace: ");
        Serial.println(client.lastError.traceback);
    }

    int rand_int;
    ok = client.call("get_rand", rand_int);

    if (ok) {
        Serial.print("Random int from server: ");
        Serial.println(rand_int);
    }

    client.notify("blink");
    Serial.println("Sent a blink notification");

    int duration_ms = 100;
    client.notify("blink", duration_ms);
    Serial.println("Sent a 100ms blink notification");

    MsgPack::object::nil_t out;
    ok = client.call("blink", out);
    Serial.print("Sent a blink RPC -> ");

    if (ok) {
        Serial.println("Server returns without issues");
    } else {
        Serial.println("Server could not handle a notification as a call");
    }

}