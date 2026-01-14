/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#ifndef ARDUINO_RPCLITE_H
#define ARDUINO_RPCLITE_H

#include "Arduino.h"

#define DECODER_BUFFER_SIZE     1024
#define RPCLITE_MAX_TRANSPORTS  3

//#define HANDLE_RPC_ERRORS
#include "transport.h"
#include "client.h"
#include "server.h"
#include "wrapper.h"
#include "dispatcher.h"
#include "decoder.h"
#include "decoder_manager.h"

#include "SerialTransport.h"

#endif //ARDUINO_RPCLITE_H
