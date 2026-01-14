# Arduino_RPClite

A MessagePack RPC library for Arduino allows to create a client/server architecture using MessagePack as the serialization format. It follows the [MessagePack-RPC protocol specification](https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md). It is designed to be lightweight and easy to use, making it suitable for embedded systems and IoT applications.


## Server

```cpp
#include <Arduino_RPClite.h>

SerialTransport transport(Serial1);
RPCServer server(transport);

int add(int a, int b){
    return a+b;
}

String loopback(String message){
    return message;
}

void setup() {
    Serial1.begin(115200);
    while(!Serial1);

    Serial.begin(9600);
    while(!Serial);

    server.bind("add", add);
    server.bind("loopback", loopback);

}

void loop() {
    server.run();
}

```


## Client

```cpp
#include <Arduino_RPClite.h>

SerialTransport transport(Serial1);
RPCClient client(transport);

void setup() {
    Serial1.begin(115200);
    while(!Serial1);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);
    while(!Serial);
}

void loop() {

    bool ok;

    String response;
    ok = client.call("loopback", response, "Sending a greeting");
    if (ok) Serial.println(str_res);

    int sum_result;
    ok = client.call("add", sum_result, 2. 3);
    if (ok) Serial.println(sum_result);     // must print 5

    // ERROR handling
    float result;
    bool ok = client.call("unbound_method", result, 10.0);
    if (!ok) {
        Serial.print("Testing Server-side exception OK. ERR code: ");
        Serial.print(client.lastError.code);
        Serial.print(" ERR trace: ");
        Serial.println(client.lastError.traceback);
    }

}

```

### Credits

This library is based on the MsgPack library by @hideakitai.
