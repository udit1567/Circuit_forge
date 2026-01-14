
#ifdef ARDUINO_SAMD_ZERO
#define MSGPACKRPC Serial    // MsgPack RPC runs on the hardware serial port (that do not disconnects on reset/upload)
#define DEBUG      SerialUSB // Debug and upload port is the native USB
#elif ARDUINO_GIGA
#define MSGPACKRPC Serial1   // MsgPack RPC runs on Serial1
#define DEBUG      SerialUSB // Debug and upload port is Serial
#elif ARDUINO_NANO_RP2040_CONNECT
#define MSGPACKRPC Serial1   // MsgPack RPC runs on Serial1
#define DEBUG      SerialUSB // Debug and upload port is Serial
#else
#error "Unsupported board"
#endif
