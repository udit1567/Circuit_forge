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

void say_hello() {
    Serial.println("Hello!");
}

float divide(float n, float d){
    return n/d;
}

int a = 10;
int b = 20;

MsgPack::Packer packer;
MsgPack::Unpacker unpacker;

MsgPack::Packer out_packer;

auto wrapped_add = wrap(add);
auto wrapped_hello = wrap(say_hello);
auto wrapped_divide = wrap(divide);

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
}

void loop() {

    packer.clear();
    MsgPack::arr_size_t param_size(2);
    packer.serialize(param_size, a, b);

    unpacker.clear();
    unpacker.feed(packer.data(), packer.size());

    out_packer.clear();

    blink_before();
    int out = (*wrapped_add)(5, 3);

    bool unpack_ok = (*wrapped_add)(unpacker, out_packer);

    Serial.print("simple call: ");
    Serial.println(out);

    if (unpack_ok){
        Serial.print("unpacker call: ");

        for (size_t i=0; i<out_packer.size(); i++){
            Serial.print(out_packer.data()[i], HEX);
            Serial.print(".");
        }

        Serial.println(" ");
    }
    
    float numerator = 1;
    float denominator = 0;

    packer.clear();
    packer.serialize(param_size, numerator, denominator);
    unpacker.clear();
    unpacker.feed(packer.data(), packer.size());
    out_packer.clear();

    bool should_be_false = (*wrapped_divide)(unpacker, out_packer);

    if (!should_be_false){
        Serial.println("RPC error call divide by zero ");
    } else {
        Serial.println("RPC error call divide by zero was handled externally nb CA 7F 80 00 00 represents inf big endian");
    }
    
    for (size_t i=0; i<out_packer.size(); i++){
        Serial.print(out_packer.data()[i], HEX);
        Serial.print(".");
    }

    Serial.println(" ");

    MsgPack::arr_size_t no_param_size(0);
    packer.clear();
    packer.serialize(no_param_size);
    unpacker.clear();
    unpacker.feed(packer.data(), packer.size());
    out_packer.clear();
    (*wrapped_hello)(unpacker, out_packer);

    for (size_t i=0; i<out_packer.size(); i++){
        Serial.print(out_packer.data()[i], HEX);
        Serial.print(".");
    }

    Serial.println(" ");

    delay(1000);
}