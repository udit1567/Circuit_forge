/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#pragma once

#ifndef BRIDGE_MONITOR_H
#define BRIDGE_MONITOR_H

#include <api/RingBuffer.h>
#include "bridge.h"

#define MON_CONNECTED_METHOD    "mon/connected"
#define MON_RESET_METHOD        "mon/reset"
#define MON_READ_METHOD         "mon/read"
#define MON_WRITE_METHOD        "mon/write"

#define DEFAULT_MONITOR_BUF_SIZE    512

template<size_t BufferSize=DEFAULT_MONITOR_BUF_SIZE>
class BridgeMonitor: public Stream {

    BridgeClass* bridge;
    RingBufferN<BufferSize> temp_buffer;
    struct k_mutex monitor_mutex{};
    bool is_connected = false;

public:
    explicit BridgeMonitor(BridgeClass& bridge): bridge(&bridge) {}

    using Print::write;

    bool begin(unsigned long _legacy_baud=0, uint16_t _legacy_config=0) {
        k_mutex_init(&monitor_mutex);

        bool bridge_started = (*bridge);
        if (!bridge_started) {
            bridge_started = bridge->begin();
        }
        return bridge_started && bridge->call(MON_CONNECTED_METHOD).result(is_connected);
    }

    explicit operator bool() const {
        return is_connected;
    }

    int read() override {
        uint8_t c;
        read(&c, 1);
        return c;
    }

    int read(uint8_t* buffer, size_t size) {
        k_mutex_lock(&monitor_mutex, K_FOREVER);
        int i = 0;
        while (temp_buffer.available() && i < size) {
            buffer[i++] = temp_buffer.read_char();
        }
        k_mutex_unlock(&monitor_mutex);
        return i;
    }

    int available() override {
        k_mutex_lock(&monitor_mutex, K_FOREVER);
        int size = temp_buffer.availableForStore();
        if (size > 0) _read(size);
        int available = temp_buffer.available();
        k_mutex_unlock(&monitor_mutex);
        return available;
    }

    int peek() override {
        k_mutex_lock(&monitor_mutex, K_FOREVER);
        if (temp_buffer.available()) {
            k_mutex_unlock(&monitor_mutex);
            return temp_buffer.peek();
        }
        k_mutex_unlock(&monitor_mutex);
        return -1;
    }

    size_t write(uint8_t c) override {
        return write(&c, 1);
    }

    size_t write(const uint8_t* buffer, size_t size) override {

        String send_buffer;

        for (size_t i = 0; i < size; ++i) {
            send_buffer += static_cast<char>(buffer[i]);
        }

        size_t written;
        const bool ret = bridge->call(MON_WRITE_METHOD, send_buffer).result(written);
        if (ret) {
            return written;
        }

        return 0;
    }

    bool reset() {
        bool res;
        bool ok = bridge->call(MON_RESET_METHOD).result(res);
        if (ok && res) {
            is_connected = false;
        }
        return (ok && res);
    }

private:
    void _read(size_t size) {

        if (size == 0) return;

        MsgPack::arr_t<uint8_t> message;
        RpcResult async_rpc = bridge->call(MON_READ_METHOD, size);

        const bool ret = async_rpc.result(message);

        if (ret) {
            k_mutex_lock(&monitor_mutex, K_FOREVER);
            for (size_t i = 0; i < message.size(); ++i) {
                temp_buffer.store_char(static_cast<char>(message[i]));
            }
            k_mutex_unlock(&monitor_mutex);
        }

        // if (async_rpc.error.code > NO_ERR) {
        //     is_connected = false;
        // }
    }

};

extern BridgeClass Bridge;
BridgeMonitor<> Monitor(Bridge);

#endif // BRIDGE_MONITOR_H