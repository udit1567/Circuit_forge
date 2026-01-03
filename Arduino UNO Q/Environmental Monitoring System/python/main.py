from arduino.app_utils import *
from arduino.app_bricks.web_ui import WebUI
from collections import deque
import time

logger = Logger("gas-monitor")

# Web UI
web_ui = WebUI()
web_ui.start()


# Rolling buffer for graph
SAMPLES_MAX = 200
samples = deque(maxlen=SAMPLES_MAX)

# Latest state
state = {
    "co2": 0.0,
    "lpg": 0.0,
    "co": 0.0,
    "smoke": 0.0,
    "temperature": 0.0,
    "pressure": 0.0
}

# REST APIs
def get_samples():
    return list(samples)

def get_gas():
    return state



# Bridge callbacks
def bmp_temperature(v):
    state["temperature"] = float(v)

def bmp_pressure(v):
    state["pressure"] = float(v)

def mq135_co2(v):
    state["co2"] = float(v)

def mq2_lpg(v):
    state["lpg"] = float(v)

def mq2_co(v):
    state["co"] = float(v)

def mq2_smoke(v):
    state["smoke"] = float(v)

    # When last value arrives → push sample + notify UI
    sample = {
        "t": time.time(),
        "co2": state["co2"],
        "co": state["co"],
        "lpg": state["lpg"]
    }

    samples.append(sample)

    web_ui.send_message("sample", sample)
    web_ui.send_message("gas", state)

# Register bridge providers
try:
    Bridge.provide("bmp_temperature", bmp_temperature)
    Bridge.provide("bmp_pressure", bmp_pressure)
    Bridge.provide("mq135_co2", mq135_co2)
    Bridge.provide("mq2_lpg", mq2_lpg)
    Bridge.provide("mq2_co", mq2_co)
    Bridge.provide("mq2_smoke", mq2_smoke)
except RuntimeError:
    pass

logger.info("Gas Bridge providers registered")

App.run()
