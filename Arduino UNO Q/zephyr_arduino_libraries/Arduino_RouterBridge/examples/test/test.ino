/*
    This file is part of the Arduino_RouterBridge library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/


#include "Arduino_RouterBridge.h"

bool x;
int i=0;

void setup() {
   Bridge.begin();
   Monitor.begin();
}

void loop() {

    // testing monitor
    Monitor.println("\r\n"+String(i));
    i++;

    // working
    Bridge.call("0_args_no_result");

    if (Bridge.call("0_args_no_result")){
      Monitor.println("0_args_no_result TRUE without result"); // returns true because there's no result
    }
    else{
      Monitor.println("0_args_no_result FALSE without result");
    }

    if (Bridge.call("0_args_bool_result")){
      Monitor.println("0_args_bool_result TRUE without result");
    }
    else{
      Monitor.println("0_args_bool_result FALSE without result"); // returns false -> check the result
    }

    x = false;
    if (Bridge.call("0_args_bool_result").result(x)){
      Monitor.println("0_args_bool_result TRUE with result: "+String(x)); // returns true - the perfect call
    }
    else{
      Monitor.println("0_args_bool_result FALSE witt result: "+String(x));
    }

    int y = -1;
    if (Bridge.call("0_args_bool_result").result(y)){
      Monitor.println("0_args_bool_result TRUE with result: "+String(y)+" (wrong result type)");
    }
    else {
      Monitor.println("0_args_bool_result FALSE with result: "+String(y)+" (wrong result type)"); // returns false - wrong type
    }

    float pow;
    RpcResult async_res = Bridge.call("1_args_float_result", 2.0, 3.0);  // passing 2 args to a function expecting 1
    if (async_res.result(pow)) {
      Monitor.println("Result of assignment and then result: "+String(pow)); // returns true, so the right result
    } else {
      Monitor.println("Error code: "+String(async_res.error.code));
      Monitor.println("Error message: "+async_res.error.traceback);
    }

    float div = 0;
    RpcResult async_res1 = Bridge.call("2_args_float_result", 2.0);  // passing 1 arg when 2 are expected
    if (async_res1.result(div)) {
      Monitor.println("Result of assignment and then result: "+String(div)); // returns true, so the right result
    } else {
      Monitor.println("Error code: "+String(async_res1.error.code));
      Monitor.println("Error message: "+async_res1.error.traceback);
    }

    div = 0;
    RpcResult async_res2 = Bridge.call("2_args_float_result", 2.0, "invalid");  // passing a wrong type arg
    if (async_res2.result(div)) {
      Monitor.println("Result of assignment and then result: "+String(div)); // returns true, so the right result
    } else {
      Monitor.println("Error code: "+String(async_res2.error.code));
      Monitor.println("Error message: "+async_res2.error.traceback);
    }

    x = false;
    RpcResult async_res3 = Bridge.call("0_args_bool_result");
    if (async_res3.result(x)) {
        Monitor.println("Result of assignment and then result: "+String(x)); // returns true, so the right result
    } else {
        Monitor.println("Error expecting bool result: "+String(async_res3.error.code));
    }

    // Avoid the following:
    // the call happens in the destructor falling back to the "no_result" case (a type mismatch here)
    RpcResult async_res4 = Bridge.call("0_args_bool_result");

    delay(1000);
}