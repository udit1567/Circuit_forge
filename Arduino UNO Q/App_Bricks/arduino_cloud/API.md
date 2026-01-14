# arduino_cloud API Reference

## Index

- Class `ArduinoCloud`

---

## `ArduinoCloud` class

```python
class ArduinoCloud(device_id: str, secret: str, server: str, port: int)
```

Arduino Cloud client for managing devices and data.

### Parameters

- **device_id** (*str*): The unique identifier for the device.
If omitted, uses ARDUINO_DEVICE_ID environment variable.
- **secret** (*str*): The password for Arduino Cloud authentication.
If omitted, uses ARDUINO_SECRET environment variable.
- **server** (*str*) (optional): The server address for Arduino Cloud (default: "iot.arduino.cc").
- **port** (*int*) (optional): The port to connect to the Arduino Cloud server (default: 8884).

### Raises

- **ValueError**: If either device_id or secret is not provided explicitly or via environment variable.

### Methods

#### `start()`

Start the Arduino IoT Cloud client.

#### `loop()`

Run a single iteration of the Arduino IoT Cloud client loop, processing commands and updating state.

#### `register(aiotobj: str | Any)`

Register a variable or object with the Arduino Cloud client.

##### Parameters

- **aiotobj** (*str | Any*): The variable name or object from which to derive the variable name to register.
- ****kwargs** (*Any*): Additional keyword arguments for registration.

