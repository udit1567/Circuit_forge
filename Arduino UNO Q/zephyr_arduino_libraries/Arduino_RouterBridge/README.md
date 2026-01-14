In this repo it will be implemented an Arduino library wrapper for RPClite to be run on Arduino UNO Q boards.

## The Bridge object ##

Including Arduino_RouterBridge.h gives the user access to a Bridge object that can be used both as a RPC client and/or server to execute and serve RPCs to/from the CPU Host running a GOLANG router.

- The Bridge object is pre-defined on Serial1 and automatically initialized inside the main setup()
- The Bridge.call method is non-blocking and returns a RpcResult async object
- RpcResult class implements a blocking .result method that waits for the RPC response and returns true if the RPC returned with no errors
- The Bridge can provide callbacks to incoming RPC requests both in a thread-unsafe and thread-safe fashion (provide & provide_safe)
- Thread-safe methods execution is granted in the main loop thread where update_safe is called. By design users cannot access .update_safe() freely
- Thread-unsafe methods are served in an update callback, whose execution is granted in a separate thread. Nonetheless users can access .update() freely with caution


```cpp
#include <Arduino_RouterBridge.h>

bool set_led(bool state) {
    digitalWrite(LED_BUILTIN, state);
    return state;
}

String greet() {
    return String("Hello Friend");
}

void setup() {

    Bridge.begin();
    Monitor.begin();

    pinMode(LED_BUILTIN, OUTPUT);

    if (!Bridge.provide("set_led", set_led)) {
        Monitor.println("Error providing method: set_led");
    } else {
        Monitor.println("Registered method: set_led");
    }

    Bridge.provide_safe("greet", greet);

}

void loop() {
    float sum;

    // CALL EXAMPLES

    // Standard chained call: Bridge.call("method", params...).result(res)
    if (!Bridge.call("add", 1.0, 2.0).result(sum)) {
        Monitor.println("Error calling method: add");
    };

    // Async call
    RpcResult async_rpc = Bridge.call("add", 3.0, 4.5);
    if (!async_rpc.result(sum)) {
        Monitor.println("Error calling method: add");
        Monitor.print("Error code: ");
        Monitor.println(async_rpc.error.code);
        Monitor.print("Error message: ");
        Monitor.println(async_rpc.error.traceback);
    }

    // Implicit boolean cast. Use with caution as in this case the call is indeed
    // executed expecting a fallback nil result (MsgPack::object::nil_t)
    if (!Bridge.call("send_greeting", "Hello Friend")) {
        Monitor.println("Error calling method: send_greeting");
    };

    // Please use notify when no reult (None, null, void, nil etc.) is expected from the opposite side
    // the following is executed immediately
    Bridge.notify("signal", 200);
}
```
