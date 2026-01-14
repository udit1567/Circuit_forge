/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#pragma once

#ifndef BRIDGE_TCP_SERVER_H
#define BRIDGE_TCP_SERVER_H

#define TCP_LISTEN_METHOD           "tcp/listen"
#define TCP_ACCEPT_METHOD           "tcp/accept"
#define TCP_CLOSE_LISTENER_METHOD   "tcp/closeListener"


#include <api/Server.h>
#include "bridge.h"
#include "tcp_client.h"

#define DEFAULT_TCP_SERVER_BUF_SIZE    512


template<size_t BufferSize=DEFAULT_TCP_SERVER_BUF_SIZE>
class BridgeTCPServer final: public Server {
    BridgeClass* bridge;
    IPAddress _addr{};
    uint16_t _port;
    bool _listening = false;
    uint32_t listener_id = 0;
    uint32_t connection_id = 0;
    bool _connected = false;
    struct k_mutex server_mutex{};

public:
    explicit BridgeTCPServer(BridgeClass& bridge, const IPAddress& addr, uint16_t port): bridge(&bridge), _addr(addr), _port(port) {}

    // explicit BridgeTCPServer(BridgeClass& bridge, uint16_t port): bridge(&bridge), _addr(INADDR_NONE), _port(port) {}

    void begin() override {
        k_mutex_init(&server_mutex);

        if (!(*bridge)) {
            while (!bridge->begin());
        }

        k_mutex_lock(&server_mutex, K_FOREVER);

        String hostname = _addr.toString();
        _listening = bridge->call(TCP_LISTEN_METHOD, hostname, _port).result(listener_id);

        k_mutex_unlock(&server_mutex);

    }

    BridgeTCPClient<BufferSize> accept() {

        if (!_listening) {
            return BridgeTCPClient<BufferSize>(*bridge, 0, false);
        }

        if (_connected) {
            return BridgeTCPClient<BufferSize>(*bridge, connection_id);
        }

        k_mutex_lock(&server_mutex, K_FOREVER);

        const bool ret = bridge->call(TCP_ACCEPT_METHOD, listener_id).result(connection_id);

        k_mutex_unlock(&server_mutex);

        if (ret) {    // connection_id 0 marks an invalid connection
            _connected = true;
            return BridgeTCPClient<BufferSize>(*bridge, connection_id);
        } else {
            _connected = false;
            // Return invalid client
            return BridgeTCPClient<BufferSize>(*bridge, 0, false);
        }

    }

    size_t write(uint8_t c) override {
        return write(&c, 1);
    }

    size_t write(const uint8_t *buf, size_t size) override {

        BridgeTCPClient<BufferSize> client = accept();

        if (client && _connected) {
            return client.write(buf, size);
        }

        return 0;
    }

    void close() {
        k_mutex_lock(&server_mutex, K_FOREVER);

        String msg;
        const bool ret = bridge->call(TCP_CLOSE_LISTENER_METHOD, listener_id).result(msg);

        if (ret) {
            _listening = false;
        }

        k_mutex_unlock(&server_mutex);
    }

    void disconnect() {
        k_mutex_lock(&server_mutex, K_FOREVER);
        _connected = false;
        connection_id = 0;
        k_mutex_unlock(&server_mutex);
    }

    bool is_listening() const {
        return _listening;
    }

    uint16_t getPort() const {
        return _port;
    }

    operator bool() const {
        return _listening;
    }

    using Print::write;

};

#endif //BRIDGE_TCP_SERVER_H