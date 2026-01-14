/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef DUMMY_TRANSPORT_H
#define DUMMY_TRANSPORT_H
#include "transport.h"

class DummyTransport: public ITransport {

    const uint8_t* _dummy_buf;
    size_t _dummy_buf_size = 0;
    size_t r_size = 0;

    public:

        DummyTransport(const uint8_t* buf, size_t size): _dummy_buf(buf), _dummy_buf_size(size){}

        void begin(){}

        bool available() override {
            return _dummy_buf_size > 0;
        }

        size_t write(const uint8_t* data, size_t size) override {
            // Mock write
            (void)data;
            return size;
        }

        size_t read(uint8_t* buffer, size_t size) override {

            size_t i;
            for (i = 0; i < size; i++){
                if ((r_size + i) == _dummy_buf_size) break;
                buffer[i] = _dummy_buf[r_size + i];
                delay(1);
            }

            r_size = r_size + i;
            return i;

        }

        size_t read_byte(uint8_t& r) override {
            uint8_t b[1];
            if (read(b, 1) != 1){
                return 0;
            };
            r = b[0];
            return 1;
        }

};

#endif  //SERIALTRANSPORT_H