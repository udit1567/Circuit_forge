# video_imageclassification API Reference

## Index

- Class `VideoImageClassification`

---

## `VideoImageClassification` class

```python
class VideoImageClassification(confidence: float, debounce_sec: float)
```

Module for image classification on a **live video stream** using a specified machine learning model.

Provides a way to react to detected classes over a video stream invoking registered actions in real-time.

### Parameters

- **confidence** (*float*): The minimum confidence level for a classification to be considered valid. Default is 0.3.
- **debounce_sec** (*float*): The minimum time in seconds between consecutive detections of the same object
to avoid multiple triggers. Default is 0 seconds.

### Raises

- **RuntimeError**: If the host address could not be resolved.

### Methods

#### `on_detect_all(callback: Callable[[dict], None])`

Register a callback invoked for **every classification event**.

This callback is useful if you want to process all classified labels in a single
place, or be notified about any classification regardless of its type.

##### Parameters

- **callback** (*Callable[[dict], None]*): A function that accepts **exactly one argument**: a dictionary of
classifications above the confidence threshold, in the form
``{"label": confidence, ...}``.

##### Raises

- **TypeError**: If `callback` is not a function.
- **ValueError**: If `callback` does not accept exactly one argument.

#### `on_detect(object: str, callback: Callable[[], None])`

Register a callback invoked when a **specific label** is classified.

The callback is triggered whenever the given label appears in the classification
results and passes the confidence and debounce filters.

##### Parameters

- **object** (*str*): The label to listen for (e.g., ``"dog"``).
- **callback** (*Callable[[], None]*): A function with **no parameters** that will be executed when the
label is detected.

##### Raises

- **TypeError**: If `callback` is not a function.
- **ValueError**: If `callback` accepts one or more parameters.

#### `start()`

Start the classification stream.

This only sets the internal running flag. You must call
`execute` in a loop or a separate thread to actually begin receiving classification results.

#### `stop()`

Stop the classification stream and release resources.

This clears the running flag. Any active `execute` loop
will exit gracefully at its next iteration.

#### `execute()`

Run the main classification loop.

Behavior:
    - Opens a WebSocket connection to the model runner.
    - Receives classification messages in real time.
    - Filters classifications below the confidence threshold.
    - Applies debounce rules before invoking callbacks.
    - Retries on transient connection errors until stopped.

##### Raises

- **ConnectionClosedOK**: Raised to exit when the server closes the connection cleanly.
- **ConnectionClosedError, TimeoutError, ConnectionRefusedError**: Logged and retried with backoff.

#### `override_threshold(value: float)`

Override the threshold for image classification model.

##### Parameters

- **value** (*float*): The new value for the threshold.

##### Raises

- **TypeError**: If the value is not a number.
- **RuntimeError**: If the model information is not available or does not support threshold override.

