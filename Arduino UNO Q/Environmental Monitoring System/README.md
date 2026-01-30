# Environmental Monitoring System 🌍

End-to-end environmental and gas monitoring system using Arduino, Linux Bridge runtime,
and a real-time Web UI.

<img width="1294" height="726" alt="Screenshot 2026-01-03 at 10 36 57 PM" src="https://github.com/user-attachments/assets/04207790-983a-40b4-9760-66b04b8b8485" />


This system reads multiple gas and environmental sensors on Arduino, transfers the data
to a Linux environment using Arduino_RouterBridge, and visualizes the data live using
WebSockets in a browser-based dashboard.



## FEATURES

- Real-time gas monitoring (PPM)
  - CO₂ (MQ135)
  - CO (MQ2)
  - LPG (MQ2)
  - Smoke (MQ2)
- Environmental sensing
  - Temperature (BMP280)
  - Pressure (BMP280)
- Exponential Moving Average (EMA) filtering on Arduino
- Rolling buffer graph (last 200 samples)
- WebSocket-based real-time UI
- Fully local edge deployment (no cloud)



## SYSTEM ARCHITECTURE

[SENSORS]
  MQ135  -> CO₂
  MQ2    -> LPG / CO / Smoke
  BMP280 -> Temperature / Pressure
      |
      v
[ARDUINO FIRMWARE]
  - Sensor acquisition
  - EMA filtering
  - Bridge.notify()
      |
      v
[ARDUINO ROUTER BRIDGE]
      |
      v
[PYTHON BACKEND - LINUX]
  - Bridge providers
  - State management
  - Rolling sample buffer
  - WebSocket server
      |
      v
[WEB UI DASHBOARD]
  - Live metric cards
  - Real-time PPM graph



## HARDWARE REQUIREMENTS

- Arduino board with Linux Bridge support
- MQ135 Gas Sensor
- MQ2 Gas Sensor
- BMP280 Sensor
- Jumper wires
- Stable power supply



## SOFTWARE REQUIREMENTS

### Arduino Side
- Arduino IDE
- Libraries:
  - Arduino_RouterBridge
  - Adafruit_BMP280
  - MQ135_XCR
  - MQGasKit
  - Wire

### Linux Side
- Python 3
- Arduino App Runtime
- Web UI Brick



## ARDUINO FIRMWARE OVERVIEW

Responsibilities:
- Read all sensors every 2 seconds
- Apply EMA filtering
- Send filtered data to Linux via Bridge

Sampling interval:
  2000 ms

EMA Formula:
  filtered = alpha * raw + (1 - alpha) * previous

Alpha value:
  0.85

Sensors transmitted:
- bmp_temperature
- bmp_pressure
- mq135_co2
- mq2_lpg
- mq2_co
- mq2_smoke



## PYTHON BACKEND OVERVIEW

Responsibilities:
- Receive sensor values from Bridge
- Maintain latest system state
- Maintain rolling buffer (max 200 samples)
- Broadcast updates to Web UI

Internal State:
- co2
- lpg
- co
- smoke
- temperature
- pressure

WebSocket Messages:
- "sample" -> graph update
- "gas"    -> live card update

REST Endpoints:
- get_samples()
- get_gas()


## WEB UI OVERVIEW

UI Sections:

1) Header
   - Environmental Monitoring System title

2) Top Grid
   - 2 x 3 metric cards
   - Live sensor values

3) Bottom Graph
   - Line graph (PPM vs time)
   - Canvas-based rendering

Graph Colors:
- Blue   -> CO₂
- Orange -> CO
- Red    -> LPG

Technologies:
- HTML5
- CSS3
- JavaScript
- Socket.IO
- Canvas API



## PROJECT STRUCTURE

project/
│
├── arduino/
│   └── gas_monitor.ino
│
├── app.py
├── app.js
├── index.html
├── style.css
├── libs/
│   └── socket.io.min.js
│
└── README.md




## DATA TIMING

- Arduino sampling interval: 2 seconds
- UI update: real-time
- Graph history length: 200 samples



## NOTES

- MQ sensors require warm-up time for stable readings
- MQ135 includes +400 ppm baseline compensation
- Designed for edge devices with low CPU usage
- Works fully offline



## AUTHOR

Built for edge IoT, sensor fusion, and real-time visualization.
