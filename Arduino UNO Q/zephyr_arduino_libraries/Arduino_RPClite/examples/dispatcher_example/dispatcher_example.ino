/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#include <Arduino_RPClite.h>

int add(int x, int y) {
    return x + y;
}

int a = 10;
int b = 20;

MsgPack::Packer packer;
MsgPack::Unpacker unpacker;

MsgPack::Packer out_packer;

RpcFunctionDispatcher<100> dispatcher;

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

void setup() {
    Serial.begin(9600);
    dispatcher.bind("add", add);
}

void loop() {

    blink_before();

    packer.clear();
    packer.serialize(a, b);

    unpacker.clear();
    unpacker.feed(packer.data(), packer.size());

    out_packer.clear();

    bool unpack_ok = dispatcher.call("add", unpacker, out_packer);

    Serial.print("OK call: ");

    if (unpack_ok){
        Serial.print("unpacker call: ");

        for (size_t i=0; i<out_packer.size(); i++){
            Serial.print(out_packer.data()[i], HEX);
            Serial.print(".");
        }

        Serial.println(" ");
    }
    

    packer.clear();
    packer.serialize(a, b);

    unpacker.clear();
    unpacker.feed(packer.data(), packer.size());

    out_packer.clear();

    unpack_ok = dispatcher.call("undeclared_fun", unpacker, out_packer);

    Serial.print("undeclared call: ");

    if (!unpack_ok){
        Serial.print("error response: ");

        for (size_t i=0; i<out_packer.size(); i++){
            Serial.print(out_packer.data()[i], HEX);
            Serial.print(".");
        }

        Serial.println(" ");
    }

    delay(1000);
}