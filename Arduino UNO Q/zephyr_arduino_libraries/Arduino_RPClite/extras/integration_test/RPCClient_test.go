//    This file is part of the Arduino_RPClite library.
//
//    Copyright (c) 2025 Arduino SA

//    This Source Code Form is subject to the terms of the Mozilla Public
//    License, v. 2.0. If a copy of the MPL was not distributed with this
//    file, You can obtain one at http://mozilla.org/MPL/2.0/.

package testsuite

import (
	"testing"
	"time"

	"github.com/stretchr/testify/require"
	"github.com/vmihailenco/msgpack/v5"
	"go.bug.st/serial"
)

func TestRPCClient(t *testing.T) {
	// Get the upload port to upload the sketch
	rpcPort, debugPort := UploadSketchAndGetRPCAndDebugPorts(t)

	// Connect to the RPC serial port
	_rpcSer, err := serial.Open(rpcPort, &serial.Mode{BaudRate: 115200})
	rpcSer := &DebugStream{Upstream: _rpcSer, Portname: rpcPort}
	require.NoError(t, err)
	t.Cleanup(func() { rpcSer.Close() })
	in := msgpack.NewDecoder(rpcSer)
	out := msgpack.NewEncoder(rpcSer)
	out.UseCompactInts(true)

	// Connect to the Debug serial port
	debugSer, err := serial.Open(debugPort, &serial.Mode{BaudRate: 115200})
	require.NoError(t, err)
	t.Cleanup(func() { debugSer.Close() })
	expectDebug := func(s string) { Expect(t, debugSer, s) }

	// Timeout fallback: close the connection after 10 seconds, if the test do not go through
	go func() {
		time.Sleep(10 * time.Second)
		rpcSer.Close()
		debugSer.Close()
	}()

	msgID := 0

	// 1: Receive an RPC call to the "mult" method with 2 arguments
	// and send back the result
	t.Run("RPCClientCallFloatArgs", func(t *testing.T) {
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(0), int8(msgID), "mult", []any{2.0, 3.0}}, arr)
		err = out.Encode([]any{1, msgID, nil, 6.0})
		require.NoError(t, err)
		expectDebug("mult(2.0, 3.0)\r\n")
		expectDebug("-> 6.00\r\n")
		msgID++
	})

	// 2: Receive an RPC call to the "mult" method with 1 argument (wrong number of arguments)
	// and send back an error with [int, string] format
	t.Run("RPCClientCallFloatArgsError", func(t *testing.T) {
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(0), int8(msgID), "mult", []any{2.0}}, arr)
		err = out.Encode([]any{1, msgID, []any{1, "missing parameter"}, nil})
		require.NoError(t, err)
		expectDebug("mult(2.0)\r\n")
		expectDebug("-> error\r\n")
		msgID++
	})

	// 3, 4: Receive an RPC call to the "or" method with 1 or 2 arguments
	// and send back the result
	t.Run("RPCClientCallBoolArgs", func(t *testing.T) {
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(0), int8(msgID), "or", []any{true, false}}, arr)
		err = out.Encode([]any{1, msgID, nil, true})
		require.NoError(t, err)
		expectDebug("or(true, false)\r\n")
		expectDebug("-> true\r\n")
		msgID++

		arr, err = in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(0), int8(msgID), "or", []any{false}}, arr)
		err = out.Encode([]any{1, msgID, nil, false})
		require.NoError(t, err)
		expectDebug("or(false)\r\n")
		expectDebug("-> false\r\n")
		msgID++
	})

	// 5: Receive an RPC call to the "mult" method with 1 argument (wrong number of arguments)
	// and send back an error with [int, string] format with a long string
	t.Run("RPCClientCallFloatArgsErrorWithLongString", func(t *testing.T) {
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(0), int8(msgID), "mult", []any{2.0}}, arr)
		err = out.Encode([]any{1, msgID, []any{2, "method get_led_state not available"}, nil})
		require.NoError(t, err)
		expectDebug("mult(2.0)\r\n")
		expectDebug("-> error\r\n")
		msgID++
	})

	// RPC: Receive an RPC call to the "mult" method with 1 argument (wrong number of arguments)
	// and send back a custom error without [int, string] format
	// t.Run("RPCClientCallFloatArgsErrorCustom", func(t *testing.T) {
	// 	arr, err := in.DecodeSlice()
	// 	require.NoError(t, err)
	// 	require.Equal(t, []any{int8(0), int8(3), "mult", []any{2.0}}, arr)
	// 	err = out.Encode([]any{1, 3, "missing parameter", nil})
	// 	require.NoError(t, err)
	// 	expectDebug("mult(2.0)\r\n")
	// 	expectDebug("-> error\r\n")
	// })
}
