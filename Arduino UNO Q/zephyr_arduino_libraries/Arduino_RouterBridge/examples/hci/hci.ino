/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#include <Arduino_RouterBridge.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("Arduino HCI Example - Read Local Version");

    if (!Bridge.begin()) {
        Serial.println("Failed to setup Bridge");
        return;
    }

    if (!HCI.begin("hci0")) {
        Serial.println("Failed to open HCI device");
        return;
    }

    Serial.println("HCI device opened successfully");

    delay(1000);
    readLocalVersion();
}

void loop() {
    // Nothing to do in loop
    delay(1000);
}

void readLocalVersion() {
    uint8_t cmd[4];
    cmd[0] = 0x01;  // HCI Command Packet Type
    cmd[1] = 0x01;  // OCF = 0x0001 (lower byte)
    cmd[2] = 0x10;  // OGF = 0x04 (0x04 << 2 = 0x10 in upper 6 bits)
    cmd[3] = 0x00;  // Parameter length = 0

    Serial.println("Sending HCI Read Local Version command...");

    int sent = HCI.send(cmd, sizeof(cmd));
    if (sent < 0) {
        Serial.println("Failed to send HCI command");
        return;
    }

    Serial.print("Sent ");
    Serial.print(sent);
    Serial.println(" bytes");

    // Wait for response with timeout
    Serial.println("Waiting for response...");
    int avail = 0;
    unsigned long startTime = millis();
    while (avail == 0 && (millis() - startTime) < 1000) {  // 1 second timeout
        avail = HCI.available();
        if (avail == 0) {
            delay(10);  // Small delay between polls
        }
    }

    Serial.print("Available bytes: ");
    Serial.println(avail);

    if (avail == 0) {
        Serial.println("Timeout: No response received");
        return;
    }

    // Read response
    uint8_t response[255];
    int received = HCI.recv(response, sizeof(response));

    if (received > 0) {
        Serial.print("Received ");
        Serial.print(received);
        Serial.println(" bytes:");

        // Print response in hex
        for (int i = 0; i < received; i++) {
            if (response[i] < 0x10) Serial.print("0");
            Serial.print(response[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        // Parse Command Complete Event
        // Event format: Packet Type, Event Code, Param Length, Num_HCI_Command_Packets, Opcode, Status, ...
        if (received >= 6 && response[0] == 0x04 && response[1] == 0x0E) {
            Serial.println("Command Complete Event received");
            Serial.print("Status: 0x");
            Serial.println(response[6], HEX);
        }
    } else {
        Serial.println("No response received");
    }
}
