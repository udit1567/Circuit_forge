/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef RPCLITE_DECODER_H
#define RPCLITE_DECODER_H

#include "MsgPack.h"
#include "transport.h"
#include "rpclite_utils.h"

using namespace RpcUtils::detail;

#define MIN_RPC_BYTES   4
#define CHUNK_SIZE      32

template<size_t BufferSize = DECODER_BUFFER_SIZE>
class RpcDecoder {

public:
    explicit RpcDecoder(ITransport& transport) : _transport(&transport) {}

    template<typename... Args>
    bool send_call(const int call_type, const MsgPack::str_t& method, uint32_t& msg_id, Args&&... args) {

        if (call_type!=CALL_MSG && call_type!=NOTIFY_MSG) return false;

        MsgPack::Packer packer;
        packer.clear();

        if (call_type == CALL_MSG){
            msg_id = _msg_id;
            MsgPack::arr_size_t call_size(REQUEST_SIZE);
            packer.serialize(call_size, call_type, msg_id, method);
        } else {
            MsgPack::arr_size_t call_size(NOTIFY_SIZE);
            packer.serialize(call_size, call_type, method);
        }

        MsgPack::arr_size_t arg_size(sizeof...(args));
        packer.serialize(arg_size, std::forward<Args>(args)...);

        if (send(reinterpret_cast<const uint8_t*>(packer.data()), packer.size()) == packer.size()){
            _msg_id++;
            return true;
        }
        return false;
    }

    template<typename RType>
    bool get_response(const uint32_t msg_id, RType& result, RpcError& error) {

        if (!packet_incoming() || _packet_type!=RESP_MSG) return false;

        MsgPack::Unpacker unpacker;
        unpacker.clear();

        if (!unpacker.feed(_raw_buffer, _packet_size)) return false;

        MsgPack::arr_size_t resp_size;
        int resp_type;
        uint32_t resp_id;

        if (!unpacker.deserialize(resp_size, resp_type, resp_id)) return false;

        // ReSharper disable once CppDFAUnreachableCode
        if (resp_id != msg_id) return false;

        // msg_id OK packet will be consumed.
        if (resp_type != RESP_MSG) {
            // This should never happen
            error.code = PARSING_ERR;
            error.traceback = "Unexpected response type";
            discard();
            return true;
        }

        if (resp_size.size() != RESPONSE_SIZE) {
            // This should never happen
            error.code = PARSING_ERR;
            error.traceback = "Unexpected RPC response size";
            discard();
            return true;
        }

        MsgPack::object::nil_t nil;
        if (unpacker.unpackable(nil)){  // No error
            if (!unpacker.deserialize(nil, result)) {
                error.code = PARSING_ERR;
                error.traceback = "Result not parsable (check type)";
                discard();
                return true;
            }
        } else {                        // RPC returned an error
            if (!unpacker.deserialize(error, nil)) {
                error.code = PARSING_ERR;
                error.traceback = "RPC Error not parsable (check type)";
                discard();
                return true;
            }
        }

        consume(_packet_size);
        reset_packet();
        return true;
    }

    bool send_response(const MsgPack::Packer& packer) const {
        return send(reinterpret_cast<const uint8_t*>(packer.data()), packer.size()) == packer.size();
    }

    MsgPack::str_t fetch_rpc_method(){

        if (!packet_incoming()){return "";}

        if (_packet_type != CALL_MSG && _packet_type != NOTIFY_MSG) {
            return ""; // No RPC
        }

        MsgPack::Unpacker unpacker;

        unpacker.clear();
        if (!unpacker.feed(_raw_buffer, _packet_size)) {    // feed should not fail at this point
            discard();
            return "";
        };

        int msg_type;
        MsgPack::str_t method;
        MsgPack::arr_size_t req_size;

        if (!unpacker.deserialize(req_size, msg_type)) {
            discard();
            return ""; // Header not unpackable
        }

        // ReSharper disable once CppDFAUnreachableCode
        if (msg_type == CALL_MSG && req_size.size() == REQUEST_SIZE) {
            uint32_t msg_id;
            if (!unpacker.deserialize(msg_id, method)) {
                discard();
                return ""; // Method not unpackable
            }
        } else if (msg_type == NOTIFY_MSG && req_size.size() == NOTIFY_SIZE) {
            if (!unpacker.deserialize(method)) {
                discard();
                return ""; // Method not unpackable
            }
        } else {
            discard();
            return ""; // Invalid request size/type
        }

        return method;

    }

    size_t get_request(uint8_t* buffer, size_t buffer_size) {

        if (_packet_type != CALL_MSG && _packet_type != NOTIFY_MSG) {
            return 0; // No RPC
        }

        return pop_packet(buffer, buffer_size);
    }

    void decode(){
        advance();
        parse_packet();
    }

    // Fill the raw buffer to its capacity
    void advance() {

        if (_transport->available() && !buffer_full()) {
            size_t bytes_read = _transport->read(_raw_buffer + _bytes_stored, BufferSize - _bytes_stored);
            _bytes_stored += bytes_read;
        }

    }

    void parse_packet(){

        if (packet_incoming()){return;}

        size_t bytes_checked = 0;
        size_t container_size;
        int type;
        MsgPack::Unpacker unpacker;

        while (bytes_checked < _bytes_stored){
            bytes_checked++;
            unpacker.clear();
            if (!unpacker.feed(_raw_buffer, bytes_checked)) continue;

            if (unpackTypedArray(unpacker, container_size, type)) {

                if (type != CALL_MSG && type != RESP_MSG && type != NOTIFY_MSG) {
                    consume(bytes_checked);
                    _discarded_packets++;
                    break; // Not a valid RPC type (could be type=WRONG_MSG)
                }

                if ((type == CALL_MSG && container_size != REQUEST_SIZE) || (type == RESP_MSG && container_size != RESPONSE_SIZE) || (type == NOTIFY_MSG && container_size != NOTIFY_SIZE)) {
                    consume(bytes_checked);
                    _discarded_packets++;
                    break; // Not a valid RPC format
                }

                _packet_type = type;
                _packet_size = bytes_checked;
                break;
            } else {
                continue;
            }

        }

    }

    bool packet_incoming() const { return _packet_size >= MIN_RPC_BYTES; }

    int packet_type() const { return _packet_type; }

    size_t get_packet_size() const { return _packet_size;}

    size_t size() const {return _bytes_stored;}

    uint32_t get_discarded_packets() const {return _discarded_packets;}

    friend class DecoderTester;

private:
    ITransport* _transport;
    uint8_t _raw_buffer[BufferSize]{};
    size_t _bytes_stored = 0;
    int _packet_type = NO_MSG;
    size_t _packet_size = 0;
    uint32_t _msg_id = 0;
    uint32_t _discarded_packets = 0;

    bool buffer_full() const { return _bytes_stored == BufferSize; }

    bool buffer_empty() const { return _bytes_stored == 0;}

    // This is a blocking send, under the assumption _transport->write will always succeed eventually
    size_t send(const uint8_t* data, const size_t size) const {

        size_t offset = 0;

        while (offset < size) {
            size_t bytes_written = _transport->write(data + offset, size - offset);
            offset += bytes_written;
        }

        return offset;
    }

    size_t pop_packet(uint8_t* buffer, size_t buffer_size) {

        if (!packet_incoming()) return 0;

        size_t packet_size = get_packet_size();
        if (packet_size > buffer_size) return 0;

        for (size_t i = 0; i < packet_size; i++) {
            buffer[i] = _raw_buffer[i];
        }

        reset_packet();
        return consume(packet_size);
    }

    void discard() {
        consume(_packet_size);
        reset_packet();
        _discarded_packets++;
    }

    void reset_packet() {
        _packet_type = NO_MSG;
        _packet_size = 0;
    }

size_t consume(size_t size, size_t offset = 0) {
    // Boundary checks
    if (offset + size > _bytes_stored || size == 0) return 0;
    
    size_t remaining_bytes = _bytes_stored - size;
    for (size_t i=offset; i<remaining_bytes; i++){
        _raw_buffer[i] = _raw_buffer[i+size];
    }

    _bytes_stored = remaining_bytes;
    return size;
}

};

#endif