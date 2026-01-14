/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef RPCLITE_DISPATCHER_H
#define RPCLITE_DISPATCHER_H

#include "wrapper.h"
#include "error.h"

struct DispatchEntry {
    MsgPack::str_t name;
    MsgPack::str_t tag;
    IFunctionWrapper* fn;
};

template<size_t N>
class RpcFunctionDispatcher {
public:

    RpcFunctionDispatcher() = default;

    template<typename F>
    bool bind(MsgPack::str_t name, F&& f, MsgPack::str_t tag="") {
        if (_count >= N) return false;

        if (isBound(name)) return false;

        _entries[_count++] = {name, tag, wrap(std::forward<F>(f))};
        return true;
    }

    bool isBound(const MsgPack::str_t& name) const {
        for (size_t i = 0; i < _count; ++i) {
            if (_entries[i].name == name) {
                return true;
            }
        }
        return false;
    }

    bool hasTag(const MsgPack::str_t& name, MsgPack::str_t& tag) const {
        for (size_t i = 0; i < _count; ++i) {
            if (_entries[i].name == name && _entries[i].tag == tag) {
                return true;
            }
        }
        return false;
    }

    bool call(const MsgPack::str_t& name, MsgPack::Unpacker& unpacker, MsgPack::Packer& packer) {
        for (size_t i = 0; i < _count; ++i) {
            if (_entries[i].name == name) {
                return (*_entries[i].fn)(unpacker, packer);
            }
        }

        // handler not found
        MsgPack::object::nil_t nil;
        packer.serialize(RpcError(FUNCTION_NOT_FOUND_ERR, name), nil);
        return false;
    }

private:
    DispatchEntry _entries[N];
    size_t _count = 0;
};

#endif