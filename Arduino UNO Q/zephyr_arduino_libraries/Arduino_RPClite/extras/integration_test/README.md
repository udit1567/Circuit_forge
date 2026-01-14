## Integration test for the RPC library

This is a test that runs on the Arduino Zero board. It could be easily extended to other boards with multiple serial ports.

### Running the test

Prerequisites:

* An Arduino Zero board, connected with both the USB ports.
* `arduino-cli`
* The platform `arduino:samd` installed through the `arduino-cli`.
* The dependencies of the RPClite library installed through the `arduino-cli`.
* A working `go` programming language compiler.

To run the test, from a terminal change directory to this folder, and run:

`go test -v`

it should compile and upload the test sketch, and perform the RPC call tests.

The output should look similar to the following:

```
RpcLite/test/RpcClientTest$ go test -v
=== RUN   TestBasicComm
Lo sketch usa 27028 byte (10%) dello spazio disponibile per i programmi. Il massimo è 262144 byte.
Le variabili globali usano 4040 byte (12%) di memoria dinamica, lasciando altri 28728 byte liberi per le variabili locali. Il massimo è 32768 byte.
Atmel SMART device 0x10010005 found
Device       : ATSAMD21G18A
Chip ID      : 10010005
Version      : v2.0 [Arduino:XYZ] Apr 11 2019 13:09:49
Address      : 8192
Pages        : 3968
Page Size    : 64 bytes
Total Size   : 248KB
Planes       : 1
Lock Regions : 16
Locked       : none
Security     : false
Boot Flash   : true
BOD          : true
BOR          : true
Arduino      : FAST_CHIP_ERASE
Arduino      : FAST_MULTI_PAGE_WRITE
Arduino      : CAN_CHECKSUM_MEMORY_BUFFER
Erase flash
done in 0.873 seconds

Write 27028 bytes to flash (423 pages)
[==============================] 100% (423/423 pages)
done in 0.155 seconds

Verify 27028 bytes of flash with checksum.
Verify successful
done in 0.026 seconds
CPU reset.
=== RUN   TestBasicComm/RPCClientCallFloatArgs
/dev/ttyACM0 READ  << 94
/dev/ttyACM0 READ  << 0001
/dev/ttyACM0 READ  << a4
/dev/ttyACM0 READ  << 6d75
/dev/ttyACM0 READ  << 6c
/dev/ttyACM0 READ  << 74
/dev/ttyACM0 READ  << 92cb
/dev/ttyACM0 READ  << 40
/dev/ttyACM0 READ  << 0000
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 READ  << 0000
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 READ  << 00cb
/dev/ttyACM0 READ  << 40
/dev/ttyACM0 READ  << 08
/dev/ttyACM0 READ  << 0000
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 READ  << 0000
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 WRITE >> 94
/dev/ttyACM0 WRITE >> 01
/dev/ttyACM0 WRITE >> 01
/dev/ttyACM0 WRITE >> c0
/dev/ttyACM0 WRITE >> cb4018000000000000
=== RUN   TestBasicComm/RPCClientCallFloatArgsError
/dev/ttyACM0 READ  << 9400
/dev/ttyACM0 READ  << 02a46d
/dev/ttyACM0 READ  << 75
/dev/ttyACM0 READ  << 6c74
/dev/ttyACM0 READ  << 91
/dev/ttyACM0 READ  << cb40
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 READ  << 0000
/dev/ttyACM0 READ  << 00
/dev/ttyACM0 READ  << 0000
/dev/ttyACM0 WRITE >> 94
/dev/ttyACM0 WRITE >> 01
/dev/ttyACM0 WRITE >> 02
/dev/ttyACM0 WRITE >> 92
/dev/ttyACM0 WRITE >> 01
/dev/ttyACM0 WRITE >> b1
/dev/ttyACM0 WRITE >> 6d697373696e6720706172616d65746572
/dev/ttyACM0 WRITE >> c0
=== RUN   TestBasicComm/RPCClientCallBoolArgs
/dev/ttyACM0 READ  << 9400
/dev/ttyACM0 READ  << 03
/dev/ttyACM0 READ  << a26f
/dev/ttyACM0 READ  << 72
/dev/ttyACM0 READ  << 92c3
/dev/ttyACM0 READ  << c2
/dev/ttyACM0 WRITE >> 94
/dev/ttyACM0 WRITE >> 01
/dev/ttyACM0 WRITE >> 03
/dev/ttyACM0 WRITE >> c0
/dev/ttyACM0 WRITE >> c3
/dev/ttyACM0 READ  << 9400
/dev/ttyACM0 READ  << 04
/dev/ttyACM0 READ  << a26f
/dev/ttyACM0 READ  << 72
/dev/ttyACM0 READ  << 91
/dev/ttyACM0 READ  << c2
/dev/ttyACM0 WRITE >> 94
/dev/ttyACM0 WRITE >> 01
/dev/ttyACM0 WRITE >> 04
/dev/ttyACM0 WRITE >> c0
/dev/ttyACM0 WRITE >> c2
/dev/ttyACM0 CLOSE
--- PASS: TestBasicComm (10.21s)
    --- PASS: TestBasicComm/RPCClientCallFloatArgs (0.03s)
    --- PASS: TestBasicComm/RPCClientCallFloatArgsError (0.03s)
    --- PASS: TestBasicComm/RPCClientCallBoolArgs (0.01s)
PASS
ok  	RpcClientZeroTest	10.216s
RpcLite/test/RpcClientTest$ 
```