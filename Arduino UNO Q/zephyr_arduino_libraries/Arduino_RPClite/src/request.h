/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef RPCLITE_REQUEST_H
#define RPCLITE_REQUEST_H

#define DEFAULT_RPC_BUFFER_SIZE  (DECODER_BUFFER_SIZE / 4)


#include "rpclite_utils.h"

template<size_t BufferSize = DEFAULT_RPC_BUFFER_SIZE>
class RPCRequest {

public:
    uint8_t buffer[BufferSize]{};
    size_t size = 0;
    int type = NO_MSG;
    uint32_t msg_id = 0;
    MsgPack::str_t method;
    MsgPack::Packer packer;
    MsgPack::Unpacker unpacker;

    static size_t get_buffer_size() {
        return BufferSize;
    }

    bool unpack_request_headers(){
        if (size == 0) return false;

        unpacker.clear();
        if (!unpacker.feed(buffer, size)) return false;

        int msg_type;
        uint32_t req_id;
        MsgPack::str_t req_method;
        MsgPack::arr_size_t req_size;

        if (!unpacker.deserialize(req_size, msg_type)) {
            return false; // Header not unpackable
        }

        if (msg_type == CALL_MSG && req_size.size() == REQUEST_SIZE) {
            if (!unpacker.deserialize(req_id, req_method)) {
                return false; // Method not unpackable
            }
        } else if (msg_type == NOTIFY_MSG && req_size.size() == NOTIFY_SIZE) {
            if (!unpacker.deserialize(req_method)) {
                return false; // Method not unpackable
            }
        } else {
            return false; // Invalid request size/type
        }

        method = req_method;
        type = msg_type;
        msg_id = req_id;

        return true;

    }

    void pack_response_headers(){
        packer.clear();
        MsgPack::arr_size_t resp_size(RESPONSE_SIZE);
        if (type == CALL_MSG) packer.serialize(resp_size, RESP_MSG, msg_id);
    }

    void reset(){
        size = 0;
        type = NO_MSG;
        msg_id = 0;
        method = "";
        unpacker.clear();
        packer.clear();
    }

};

#endif //RPCLITE_REQUEST_H