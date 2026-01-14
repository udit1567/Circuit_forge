/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef RPCLITE_CLIENT_H
#define RPCLITE_CLIENT_H
#include "error.h"
#include "decoder_manager.h"


class RPCClient {
    RpcDecoder<>* decoder;

public:
    RpcError lastError;

    explicit RPCClient(ITransport& t) : decoder(&RpcDecoderManager<>::getDecoder(t)) {}

    template<typename... Args>
    void notify(const MsgPack::str_t& method, Args&&... args)  {
        uint32_t _id;
        decoder->send_call(NOTIFY_MSG, method, _id, std::forward<Args>(args)...);
    }

    template<typename RType, typename... Args>
    bool call(const MsgPack::str_t& method, RType& result, Args&&... args) {

        uint32_t msg_id_wait;

        if(!send_rpc(method, msg_id_wait, std::forward<Args>(args)...)) {
            lastError.code = GENERIC_ERR;
            lastError.traceback = "Failed to send RPC call";
            return false;
        }

        // blocking call
        RpcError tmp_error;
        while (!get_response(msg_id_wait, result, tmp_error)) {
            //delay(1);
        }

        return (lastError.code == NO_ERR);

    }

    template<typename... Args>
    bool send_rpc(const MsgPack::str_t& method, uint32_t& wait_id, Args&&... args) {
        uint32_t msg_id;
        if (decoder->send_call(CALL_MSG, method, msg_id, std::forward<Args>(args)...)) {
            wait_id = msg_id;
            return true;
        }
        return false;
    }

    template<typename RType>
    bool get_response(const uint32_t wait_id, RType& result, RpcError& error) {
        decoder->decode();

        if (decoder->get_response(wait_id, result, error)) {
            lastError.copy(error);
            return true;
        }
        return false;
    }

    uint32_t get_discarded_packets() const {return decoder->get_discarded_packets();}

};

#endif //RPCLITE_CLIENT_H
