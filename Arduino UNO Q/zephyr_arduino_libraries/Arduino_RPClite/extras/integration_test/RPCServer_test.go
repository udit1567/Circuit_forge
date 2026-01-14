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

func TestRPCServer(t *testing.T) {
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

	// 1: Send an RPC call to the "add" method with 2 arguments
	// and get back the result
	t.Run("RPCServerCallIntArgs", func(t *testing.T) {
		err = out.Encode([]any{0, 1, "add", []any{2, 3}})
		require.NoError(t, err)
		expectDebug("add(2, 3)\r\n")
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(1), int8(1), nil, int8(5)}, arr)
	})

	// 2, 3: Send the same RPC call with 1 and 3 arguments, and get back the error
	t.Run("RPCServerCallWrongIntArgsCount", func(t *testing.T) {
		err = out.Encode([]any{0, 2, "add", []any{2}})
		require.NoError(t, err)
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{
			int8(1),
			int8(2),
			[]any{uint8(253), "Missing call parameters (WARNING: Default param resolution is not implemented)"},
			nil,
		}, arr)

		err = out.Encode([]any{0, 3, "add", []any{2, 3, 4}})
		require.NoError(t, err)
		arr, err = in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{
			int8(1),
			int8(3),
			[]any{uint8(253), "Too many parameters"},
			nil,
		}, arr)
	})

	// 4: Send an RPC call to the "greet" method
	t.Run("RPCServerCallNoArgsReturnString", func(t *testing.T) {
		err = out.Encode([]any{0, 4, "greet", []any{}})
		require.NoError(t, err)
		expectDebug("greet()\r\n")
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(1), int8(4), nil, "Hello World!"}, arr)
	})

	// 5: Send an RPC call to the "loopback" method with 1 string argument
	t.Run("RPCServerCallStringArgsReturnString", func(t *testing.T) {
		err = out.Encode([]any{0, 5, "loopback", []any{"Hello World!"}})
		require.NoError(t, err)
		expectDebug("loopback(\"Hello World!\")\r\n")
		arr, err := in.DecodeSlice()
		require.NoError(t, err)
		require.Equal(t, []any{int8(1), int8(5), nil, "Hello World!"}, arr)
	})
}
