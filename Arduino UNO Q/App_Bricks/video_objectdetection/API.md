# video_objectdetection API Reference

## Index

- Class `VideoObjectDetection`

---

## `VideoObjectDetection` class

```python
class VideoObjectDetection(confidence: float, debounce_sec: float)
```

Module for object detection on a **live video stream** using a specified machine learning model.

This brick:
  - Connects to a model runner over WebSocket.
  - Parses incoming classification messages with bounding boxes.
  - Filters detections by a configurable confidence threshold.
  - Debounces repeated triggers of the same label.
  - Invokes per-label callbacks and/or a catch-all callback.

### Parameters

- **confidence** (*float*): Confidence level for detection. Default is 0.3 (30%).
- **debounce_sec** (*float*): Minimum seconds between repeated detections of the same object. Default is 0 seconds.

### Raises

- **RuntimeError**: If the host address could not be resolved.

### Methods

#### `on_detect(object: str, callback: Callable[[], None])`

Register a callback invoked when a **specific label** is detected.

##### Parameters

- **object** (*str*): The label of the object to check for in the classification results.
- **callback** (*Callable[[], None]*): A function with **no parameters**.

##### Raises

- **TypeError**: If `callback` is not a function.
- **ValueError**: If `callback` accepts any parameters.

#### `on_detect_all(callback: Callable[[dict], None])`

Register a callback invoked for **every detection event**.

This is useful to receive a consolidated dictionary of detections for each frame.

##### Parameters

- **callback** (*Callable[[dict], None]*): A function that accepts **one dict argument** with
the shape `{label: confidence, ...}`.

##### Raises

- **TypeError**: If `callback` is not a function.
- **ValueError**: If `callback` does not accept exactly one argument.

#### `start()`

Start the video object detection process.

#### `stop()`

Stop the video object detection process.

#### `execute()`

Connect to the model runner and process messages until `stop` is called.

Behavior:
    - Establishes a WebSocket connection to the runner.
    - Parses ``"hello"`` messages to capture model metadata and optionally
      performs a threshold override to align the runner with the local setting.
    - Parses ``"classification"`` messages, filters detections by confidence,
      applies debounce, then invokes registered callbacks.
    - Retries on transient WebSocket errors while running.

##### Raises

- **ConnectionClosedOK**: Propagated to exit cleanly when the server closes the connection.
- **ConnectionClosedError, TimeoutError, ConnectionRefusedError**: Logged and retried with a short backoff while running.

#### `override_threshold(value: float)`

Override the threshold for object detection model.

##### Parameters

- **value** (*float*): The new value for the threshold in the range [0.0, 1.0].

##### Raises

- **TypeError**: If the value is not a number.
- **RuntimeError**: If the model information is not available or does not support threshold override.

