# A python sketch that uses RPC Bridge to test the server.ino

import time
from arduino.app_utils import *


def log(msg):
    with open("./log.log", "a") as f:
        f.write(str(msg) + "\n")

def main():
    res = Bridge.call("tcp/connect", "127.0.0.1", 5678)
    log(f"Connection attempt id: {res}")

    written = Bridge.call("tcp/write", res, "Hello friend")
    log(f"Written msg of len: {written}")

    time.sleep(1)

    ok = Bridge.call("tcp/close", res)
    log(f"Closed connection: {ok}")

if __name__ == "__main__":

    while True:
        main()
        time.sleep(1)
