/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef RPCLITE_ERROR_H
#define RPCLITE_ERROR_H

#include <utility>

#include "MsgPack.h"

#define NO_ERR                   0x00
#define PARSING_ERR              0xFC
#define MALFORMED_CALL_ERR       0xFD
#define FUNCTION_NOT_FOUND_ERR   0xFE
#define GENERIC_ERR              0xFF


struct RpcError {
    int code;
    MsgPack::str_t traceback;

    RpcError() {
        code = NO_ERR;
        traceback = "";
    }

    RpcError(const int c, MsgPack::str_t tb)
        : code(c), traceback(std::move(tb)) {}

    void copy(const RpcError& err) {
        code = err.code;
        traceback = err.traceback;
    }

    MSGPACK_DEFINE(code, traceback); // -> [code, traceback]
};

#endif