/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#pragma once

#ifndef ROUTER_BRIDGE_H
#define ROUTER_BRIDGE_H

#define RESET_METHOD "$/reset"
#define BIND_METHOD "$/register"
//#define BRIDGE_ERROR "$/bridgeLog"

#define UPDATE_THREAD_STACK_SIZE    500
#define UPDATE_THREAD_PRIORITY      5

#define DEFAULT_SERIAL_BAUD         115200

#include <zephyr/kernel.h>
#include <Arduino_RPClite.h>


void updateEntryPoint(void *, void *, void *);

template<typename... Args>
class RpcResult {

public:
    RpcError error;

    RpcResult(const MsgPack::str_t& m, RPCClient* c, struct k_mutex* rm, struct k_mutex* wm, Args&&... args): method(m), client(c), read_mutex(rm), write_mutex(wm), callback_params(std::forward_as_tuple(std::forward<Args>(args)...)) {}

    template<typename RType> bool result(RType& result) {
        if (_executed) return error.code == NO_ERR;

        while (true) {
            if (k_mutex_lock(write_mutex, K_MSEC(10)) == 0) {
                std::apply([this](const auto&... elems) {
                    client->send_rpc(method, msg_id_wait, elems...);
                }, callback_params);
                k_mutex_unlock(write_mutex);
                break;
            } else {
                k_yield();
            }
        }

        while(true) {
            if (k_mutex_lock(read_mutex, K_MSEC(10)) == 0 ) {
                if (client->get_response(msg_id_wait, result, error)) {
                    k_mutex_unlock(read_mutex);
                    // if (error.code == PARSING_ERR) {
                    //     k_mutex_lock(write_mutex, K_FOREVER);
                    //     client->notify(BRIDGE_ERROR, error.traceback);
                    //     k_mutex_unlock(write_mutex);
                    // }
                    break;
                }
                k_mutex_unlock(read_mutex);
                k_msleep(1);
            } else {
                k_yield();
            }
        }
        _executed = true;
        return error.code == NO_ERR;
    }

    bool result() {
        MsgPack::object::nil_t nil;
        return result(nil);
    }

    ~RpcResult(){
        result();
    }

    operator bool() {
        return result();
    }

private:
    uint32_t msg_id_wait{};
    bool _executed = false;

    MsgPack::str_t method;
    RPCClient* client;
    struct k_mutex* read_mutex;
    struct k_mutex* write_mutex;
    std::tuple<Args...> callback_params;
};

class BridgeClass {

    RPCClient* client = nullptr;
    RPCServer* server = nullptr;
    HardwareSerial* serial_ptr = nullptr;
    ITransport* transport = nullptr;

    struct k_mutex read_mutex{};
    struct k_mutex write_mutex{};

    k_tid_t upd_tid{};
    k_thread_stack_t *upd_stack_area{};
    struct k_thread upd_thread_data{};

    bool started = false;

public:

    explicit BridgeClass(HardwareSerial& serial) {
        serial_ptr = &serial;
    }

    operator bool() const {
        return started;
    }

    // Initialize the bridge
    bool begin(unsigned long baud=DEFAULT_SERIAL_BAUD) {
        serial_ptr->begin(baud);
        transport = new SerialTransport(*serial_ptr);

        k_mutex_init(&read_mutex);
        k_mutex_init(&write_mutex);

        client = new RPCClient(*transport);
        server = new RPCServer(*transport);

        upd_stack_area = k_thread_stack_alloc(UPDATE_THREAD_STACK_SIZE, 0);
        upd_tid = k_thread_create(&upd_thread_data, upd_stack_area,
                                UPDATE_THREAD_STACK_SIZE,
                                updateEntryPoint,
                                NULL, NULL, NULL,
                                UPDATE_THREAD_PRIORITY, 0, K_NO_WAIT);

        bool res;
        call(RESET_METHOD).result(res);
        if (res) {
            started = true;
        }
        return res;
    }

    template<typename F>
    bool provide(const MsgPack::str_t& name, F&& func) {
        bool res;
        if (!call(BIND_METHOD, name).result(res)) {
            return false;
        }
        return server->bind(name, func);
    }

    template<typename F>
    bool provide_safe(const MsgPack::str_t& name, F&& func) {
        bool res;
        if (!call(BIND_METHOD, name).result(res)) {
            return false;
        }

        return server->bind(name, func, "__safe__");

    }

    void update() {

        // Lock read mutex
        if (k_mutex_lock(&read_mutex, K_MSEC(10)) != 0 ) return;

        RPCRequest<> req;
        if (!server->get_rpc(req)) {
            k_mutex_unlock(&read_mutex);
            k_msleep(1);
            return;
        }

        k_mutex_unlock(&read_mutex);

        server->process_request(req);

        // Lock write mutex
        while (true) {

            if (k_mutex_lock(&write_mutex, K_MSEC(10)) == 0){
                server->send_response(req);
                k_mutex_unlock(&write_mutex);
                break;
            } else {
                k_yield();
            }

        }

    }

    template<typename... Args>
    RpcResult<Args...> call(const MsgPack::str_t& method, Args&&... args) {
       return RpcResult<Args...>(method, client, &read_mutex, &write_mutex, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void notify(const MsgPack::str_t method, Args&&... args)  {
        while (true) {
            if (k_mutex_lock(&write_mutex, K_MSEC(10)) == 0) {
                client->notify(method, std::forward<Args>(args)...);
                k_mutex_unlock(&write_mutex);
                break;
            }
            k_yield();
        }
    }

private:

    void update_safe() {

        // Lock read mutex
        if (k_mutex_lock(&read_mutex, K_MSEC(10)) != 0 ) return;

        RPCRequest<> req;
        if (!server->get_rpc(req, "__safe__")) {
            k_mutex_unlock(&read_mutex);
            k_msleep(1);
            return;
        }

        k_mutex_unlock(&read_mutex);

        server->process_request(req);

        // Lock write mutex
        while (true) {

            if (k_mutex_lock(&write_mutex, K_MSEC(10)) == 0){
                server->send_response(req);
                k_mutex_unlock(&write_mutex);
                break;
            } else {
                k_yield();
            }

        }

    }

    friend class BridgeClassUpdater;

};

class BridgeClassUpdater {
public:
    static void safeUpdate(BridgeClass* bridge) {
        if (*bridge) {
            bridge->update_safe();
        }
    }

private:
    BridgeClassUpdater() = delete; // prevents instantiation
};

BridgeClass Bridge(Serial1);

inline void updateEntryPoint(void *, void *, void *){
    while (true) {
        if (Bridge) {
            Bridge.update();
        }
        k_yield();
    }
}

static void safeUpdate(){
    BridgeClassUpdater::safeUpdate(&Bridge);
}

// leave as is
void __loopHook(void){
    k_yield();
    safeUpdate();
}

#endif // ROUTER_BRIDGE_H