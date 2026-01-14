# This file is part of the Arduino_RPClite library.
#
# Copyright (c) 2025 Arduino SA
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from serial_client import SerialClient

PORT = '/dev/ttySTM0'

client = SerialClient(port=PORT, baudrate=115200)


client.call("add", 10)       # not enough args

client.call("add", 15, 7)   # just enough args

client.call("add", 1, 2, 3) # too many args

client.call("greet")

client.notify("add", 5, 9)

client.notify("greet")