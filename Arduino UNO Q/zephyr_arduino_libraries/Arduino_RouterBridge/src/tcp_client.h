/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#pragma once

#ifndef BRIDGE_TCP_CLIENT_H
#define BRIDGE_TCP_CLIENT_H

#define TCP_CONNECT_METHOD          "tcp/connect"
#define TCP_CONNECT_SSL_METHOD      "tcp/connectSSL"
#define TCP_CLOSE_METHOD            "tcp/close"
#define TCP_WRITE_METHOD            "tcp/write"
#define TCP_READ_METHOD             "tcp/read"

#include <api/RingBuffer.h>
#include <api/Client.h>
#include "bridge.h"

#define DEFAULT_TCP_CLIENT_BUF_SIZE    512


template<size_t BufferSize=DEFAULT_TCP_CLIENT_BUF_SIZE>
class BridgeTCPClient : public Client {

    BridgeClass* bridge;
    uint32_t connection_id{};
    RingBufferN<BufferSize> temp_buffer;
    struct k_mutex client_mutex{};
    bool _connected = false;

public:
    explicit BridgeTCPClient(BridgeClass& bridge): bridge(&bridge) {}

    BridgeTCPClient(BridgeClass& bridge, uint32_t connection_id, bool connected=true): bridge(&bridge), connection_id(connection_id), _connected {connected} {}

    bool begin() {
        k_mutex_init(&client_mutex);
        if (!(*bridge)) {
            return bridge->begin();
        }
        return true;
    }

    int connect(IPAddress ip, uint16_t port) override {
        return connect(ip.toString().c_str(), port);
    }

    int connect(const char *host, uint16_t port) override {

        if (_connected) return 0;

        String hostname = host;

        k_mutex_lock(&client_mutex, K_FOREVER);

        const bool resp = bridge->call(TCP_CONNECT_METHOD, hostname, port).result(connection_id);

        if (!resp) {
            _connected = false;
            k_mutex_unlock(&client_mutex);
            return -1;
        }
        _connected = true;

        k_mutex_unlock(&client_mutex);

        return 0;
    }

    int connectSSL(const char *host, uint16_t port, const char *ca_cert) {

        if (_connected) return 0;

        String hostname = host;
        String ca_cert_str = ca_cert;

        k_mutex_lock(&client_mutex, K_FOREVER);

        const bool resp = bridge->call(TCP_CONNECT_SSL_METHOD, hostname, port, ca_cert_str).result(connection_id);

        if (!resp) {
            _connected = false;
            k_mutex_unlock(&client_mutex);
            return -1;
        }
        _connected = true;

        k_mutex_unlock(&client_mutex);
        return 0;
    }

    uint32_t getId() const {
        return connection_id;
    }

    size_t write(uint8_t c) override {
        return write(&c, 1);
    }

    size_t write(const uint8_t *buf, size_t size) override {

        if (!_connected) return 0;

        MsgPack::arr_t<uint8_t> payload;

        for (size_t i = 0; i < size; ++i) {
            payload.push_back(buf[i]);
        }

        size_t written;
        const bool ret = bridge->call(TCP_WRITE_METHOD, connection_id, payload).result(written);
        if (ret) {
            return written;
        }

        return 0;
    }

    int available() override {
        k_mutex_lock(&client_mutex, K_FOREVER);
        const int size = temp_buffer.availableForStore();
        if (size > 0) _read(size);
        const int _available = temp_buffer.available();
        k_mutex_unlock(&client_mutex);
        return _available;
    }

    int read() override {
        uint8_t c;
        read(&c, 1);
        return c;
    }

    int read(uint8_t *buf, size_t size) override {
        k_mutex_lock(&client_mutex, K_FOREVER);
        int i = 0;
        while (temp_buffer.available() && i < size) {
            buf[i++] = temp_buffer.read_char();
        }
        k_mutex_unlock(&client_mutex);
        return i;
    }

    int peek() override {
        k_mutex_lock(&client_mutex, K_FOREVER);
        if (temp_buffer.available()) {
            k_mutex_unlock(&client_mutex);
            return temp_buffer.peek();
        }
        k_mutex_unlock(&client_mutex);
        return -1;
    }

    void flush() override {
        // No-op: flush is implemented for Client subclasses using an output buffer
    }

    void close() {
        stop();
    }

    void stop() override {
        k_mutex_lock(&client_mutex, K_FOREVER);
        String msg;
        const bool resp = bridge->call(TCP_CLOSE_METHOD, connection_id).result(msg);
        if (resp) {
            _connected = false;
        }
        k_mutex_unlock(&client_mutex);
    }

    uint8_t connected() override {
        if (_connected) return 1;
        return 0;
    }

    operator bool() override {
        return available() || connected();
    }

    //friend class BridgeTCPServer;

    using Print::write;

private:
    void _read(size_t size) {

        if (size == 0 || !_connected) return;

        k_mutex_lock(&client_mutex, K_FOREVER);

        MsgPack::arr_t<uint8_t> message;
        RpcResult async_rpc = bridge->call(TCP_READ_METHOD, connection_id, size);

        const bool ret = async_rpc.result(message);

        if (ret) {
            for (size_t i = 0; i < message.size(); ++i) {
                temp_buffer.store_char(static_cast<char>(message[i]));
            }
        }

        if (async_rpc.error.code > NO_ERR) {
            _connected = false;
        }

        k_mutex_unlock(&client_mutex);
    }

};


#endif //BRIDGE_TCP_CLIENT_H
