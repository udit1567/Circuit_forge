# This file is part of the Arduino_RPClite library.
#
# Copyright (c) 2025 Arduino SA
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import serial
import msgpack
import threading
from io import BytesIO

REQUEST = 0
RESPONSE = 1
NOTIFY = 2

GENERIC_EXCEPTION = 0xff

class SerialServer:
    def __init__(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        self.callbacks = {}
        self.running = False

    def register_callback(self, command, func):
        """Register a callback for a specific command key"""
        self.callbacks[command] = func

    def on_request(self, msg_id, command, args):
        """Execute the callback and respond"""
        try:
            result = self.callbacks[command](*args)
            return [RESPONSE, msg_id, None, result]
        except Exception as e:
            return [RESPONSE, msg_id, [GENERIC_EXCEPTION, str(e)], None]

    def on_notify(self, command, args):
        """Execute the callback"""
        try:
            self.callbacks[command](*args)
        except Exception as e:
            print(f"Exception on notification... the client will never know {str(e)}")

    def handle_message(self, message) -> bytes | None:
        """Process incoming messages"""
        msgsize = len(message)
        if msgsize != 4 and msgsize != 3:
            raise Exception("Invalid MessagePack-RPC protocol: message = {0}".format(message))

        msgtype = message[0]
        if msgtype == REQUEST:
            response = self.on_request(message[1], message[2], message[3])
        elif msgtype == RESPONSE:
            raise Exception("Server receiving RESPONSE not implemented")
            # response = self.on_response(message[1], message[2], message[3])
        elif msgtype == NOTIFY:
            self.on_notify(message[1], message[2])
            return None
        else:
            raise Exception("Unknown message type: type = {0}".format(msgtype))

        return msgpack.packb(response)

    def start(self):
        """Start the serial server loop"""
        self.running = True
        threading.Thread(target=self._run, daemon=True).start()

    def _run(self):
        while self.running:
            try:
                data = self.ser.read(1024)
                if data:
                    unpacker = msgpack.Unpacker(BytesIO(data))
                    for message in unpacker:
                        response = self.handle_message(message)
                        if response is not None:
                            self.ser.write(response)
            except Exception as e:
                print(f"Error: {e}")
        print("Server stopped")

    def stop(self):
        self.running = False
        self.ser.close()
