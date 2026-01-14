/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#pragma once
#ifndef RPCLITE_UTILS_H
#define RPCLITE_UTILS_H

#include <tuple>
#include <utility>

namespace RpcUtils {
namespace detail {

#define TYPE_ERROR      (-1)
#define WRONG_MSG       (-2)
#define NO_MSG          (-1)
#define CALL_MSG        0
#define RESP_MSG        1
#define NOTIFY_MSG      2

#define REQUEST_SIZE    4
#define RESPONSE_SIZE   4
#define NOTIFY_SIZE     3

///////////////////////////////////////
/// --- deserialization helpers --- ///
///////////////////////////////////////

inline bool unpackObject(MsgPack::Unpacker& unpacker);


inline bool unpackTypedArray(MsgPack::Unpacker& unpacker, size_t& size, int& type) {

    if (!unpacker.isArray()) {
        return false; // Not an array
    }

    MsgPack::arr_size_t sz;
    unpacker.deserialize(sz);
    int rpc_type;

    size = 0;
    for (size_t i=0; i<sz.size(); i++){
        if (i==0) {
            if (unpacker.isInt() || unpacker.isUInt()) {
                unpacker.deserialize(rpc_type);
                type = rpc_type;
                size++;
                continue; // the First element must be the type
            } else {
                type = WRONG_MSG; // Not a valid type
            }
        }

        if (unpackObject(unpacker)){
            size++;
        } else {
            return false;
        }
    }

    return true;

}

inline bool unpackArray(MsgPack::Unpacker& unpacker, size_t& size) {

    if (!unpacker.isArray()) {
        return false; // Not an array
    }

    MsgPack::arr_size_t sz;
    unpacker.deserialize(sz);

    size = 0;
    for (size_t i=0; i<sz.size(); i++){
        if (unpackObject(unpacker)){
            size++;
        } else {
            return false;
        }
    }

    return true;

}

inline bool unpackMap(MsgPack::Unpacker& unpacker, size_t& size) {

    if (!unpacker.isMap()) {
        return false; // Not a map
    }

    MsgPack::map_size_t sz;
    unpacker.deserialize(sz);

    size = 0;
    for (size_t i=0; i<sz.size(); i++){
        if (unpackObject(unpacker) && unpackObject(unpacker)){  // must unpack key&value
            size++;
        } else {
            return false;
        }
    }

    return true;

}

inline bool unpackObject(MsgPack::Unpacker& unpacker){

    if (unpacker.isNil()){
        static MsgPack::object::nil_t nil;
        return unpacker.deserialize(nil);
    }
    if (unpacker.isBool()){
        static bool b;
        return unpacker.deserialize(b);
    }
    if (unpacker.isUInt() || unpacker.isInt()){
        static int integer;
        return unpacker.deserialize(integer);
    }
    if (unpacker.isFloat32()){
        static float num32;
        return unpacker.deserialize(num32);
    }
    if (unpacker.isFloat64()){
        static double num64;
        return unpacker.deserialize(num64);
    }
    if (unpacker.isStr()){
        static MsgPack::str_t string;
        return unpacker.deserialize(string);
    }
    if (unpacker.isBin()){
        static MsgPack::bin_t<uint8_t> bytes;
        return unpacker.deserialize(bytes);
    }
    if (unpacker.isArray()){
        static size_t arr_sz;
        return unpackArray(unpacker, arr_sz);
    }
    if (unpacker.isMap()){
        static size_t map_sz;
        return unpackMap(unpacker, map_sz);
    }
    if (unpacker.isFixExt() || unpacker.isExt()){
        static MsgPack::object::ext e;
        return unpacker.deserialize(e);
    }
    if (unpacker.isTimestamp()){
        static MsgPack::object::timespec t;
        return unpacker.deserialize(t);
    }

    return false;
}

template<typename T>
int deserialize_single(MsgPack::Unpacker& unpacker, T& value) {
    if (!unpacker.unpackable(value)) return TYPE_ERROR;
    unpacker.deserialize(value);
    return 0;
}


/////////////////////////////
/// --- tuple helpers --- ///
/////////////////////////////

template<std::size_t I = 0, typename... Ts>
typename std::enable_if<I == sizeof...(Ts), int>::type
deserialize_tuple(MsgPack::Unpacker&, std::tuple<Ts...>&) {
    return 0;
}

template<std::size_t I = 0, typename... Ts>
typename std::enable_if<I < sizeof...(Ts), int>::type
deserialize_tuple(MsgPack::Unpacker& unpacker, std::tuple<Ts...>& out) {
    const int res = deserialize_single(unpacker, std::get<I>(out));
    if (res==TYPE_ERROR) return TYPE_ERROR-I;
    return deserialize_tuple<I + 1>(unpacker, out);
}

// Helper to invoke a function with a tuple of arguments
template<typename F, typename Tuple, std::size_t... I>
auto invoke_with_tuple(F&& f, Tuple&& t, arx::stdx::index_sequence<I...>)
    -> decltype(f(std::get<I>(std::forward<Tuple>(t))...)) {
    return f(std::get<I>(std::forward<Tuple>(t))...);
}

} // namespace detail
} // namespace RpcUtils

#endif //RPCLITE_UTILS_H