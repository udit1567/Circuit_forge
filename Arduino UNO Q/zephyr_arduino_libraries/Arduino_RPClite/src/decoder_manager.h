/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

// This is a static implementation of the decoder manager

#ifndef RPCLITE_DECODER_MANAGER_H
#define RPCLITE_DECODER_MANAGER_H

#include <array>
#include "transport.h"
#include "decoder.h"

template<size_t MaxTransports = RPCLITE_MAX_TRANSPORTS>
class RpcDecoderManager {
public:
    static RpcDecoder<>& getDecoder(ITransport& transport) {
        for (auto& entry : decoders_) {
            if (entry.transport == &transport) {
                return *entry.decoder;
            }

            if (entry.transport == nullptr) {
                entry.transport = &transport;
                // In-place construct
                entry.decoder = new (&entry.decoder_storage.instance) RpcDecoder<>(transport);
                decoders_count++;
                return *entry.decoder;
            }
        }

        // No slot available — simple trap for now
        while (true);
    }

    static size_t getDecodersCount() {
        return decoders_count;
    }

private:
    struct DecoderStorage {
        union {
            RpcDecoder<> instance;
            uint8_t raw[sizeof(RpcDecoder<>)]{};
        };

        DecoderStorage() {}
        ~DecoderStorage() {}
    };

    struct Entry {
        ITransport* transport = nullptr;
        RpcDecoder<>* decoder = nullptr;
        DecoderStorage decoder_storage;
    };

    static std::array<Entry, MaxTransports> decoders_;
    static size_t decoders_count;
};

// Definition of the static member
template<size_t MaxTransports>
std::array<typename RpcDecoderManager<MaxTransports>::Entry, MaxTransports> RpcDecoderManager<MaxTransports>::decoders_;

template<size_t MaxTransports>
size_t RpcDecoderManager<MaxTransports>::decoders_count = 0;

#endif //RPCLITE_DECODER_MANAGER_H