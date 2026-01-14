/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#pragma once
#ifndef RPCLITE_DECODER_TESTER_H
#define RPCLITE_DECODER_TESTER_H

class DecoderTester {
  
  RpcDecoder<>& decoder;

public:

  DecoderTester(RpcDecoder<>& _d): decoder(_d){}

  void crop_bytes(size_t size, size_t offset){
    decoder.consume(size, offset);
  }

  void print_raw_buf(){

    Serial.print("Decoder raw buffer content: ");

    for (size_t i = 0; i < decoder._bytes_stored; i++) {

        Serial.print(decoder._raw_buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println("");
  }

};

#endif // RPCLITE_DECODER_TESTER_H