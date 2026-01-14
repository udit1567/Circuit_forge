/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#include <Arduino_RPClite.h>
#include "serial_ports.h"

SerialTransport transport(&MSGPACKRPC);
RPCServer server(transport);

int add(int a, int b) {
    DEBUG.print("add(");
    DEBUG.print(a);
    DEBUG.print(", ");
    DEBUG.print(b);
    DEBUG.println(")");
    return a+b;
}

MsgPack::str_t greet() {
    DEBUG.println("greet()");
    return MsgPack::str_t ("Hello World!");
}

MsgPack::str_t loopback(MsgPack::str_t message){
    DEBUG.print("loopback(\"");
    DEBUG.print(message);
    DEBUG.println("\")");
    return message;
}

void setup() {
    MSGPACKRPC.begin(115200);
    DEBUG.begin(115200);
    transport.begin();
    server.bind("add", add);
    server.bind("greet", greet);
    server.bind("loopback", loopback);

    while (!DEBUG) { /* WAIT for serial port to connect */ }
}

void loop() {
    server.run();
}
