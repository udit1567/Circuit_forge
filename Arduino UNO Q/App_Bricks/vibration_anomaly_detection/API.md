# vibration_anomaly_detection API Reference

## Index

- Class `VibrationAnomalyDetection`

---

## `VibrationAnomalyDetection` class

```python
class VibrationAnomalyDetection(anomaly_detection_threshold: float)
```

Detect vibration anomalies from accelerometer time-series using a pre-trained

Edge Impulse model.

This Brick buffers incoming samples into a sliding window sized to the model’s
`input_features_count`, runs inference when a full window is available, extracts
the **anomaly score**, and (optionally) invokes a user-registered callback when
the score crosses a configurable threshold.

Notes:
    - Requires an active Edge Impulse runner; model info is fetched at init.
    - The window size equals the model’s `input_features_count`; samples pushed
      via `accumulate_samples()` are flattened before inference.
    - The expected **units, axis order, and sampling rate** must match those
      used during model training (e.g., m/s² vs g, [ax, ay, az], 100 Hz).
    - A single callback is supported at a time (thread-safe registration).

### Parameters

- **anomaly_detection_threshold** (*float*): Threshold applied to the model’s
anomaly score to decide whether to trigger the registered callback.
Typical starting point is 1.0; tune based on your dataset.

### Raises

- **ValueError**: If the Edge Impulse runner is unreachable, or if the model
info is missing/invalid (e.g., non-positive `frequency` or
`input_features_count`).

### Methods

#### `accumulate_samples(sensor_samples: Iterable[float])`

Append one or more accelerometer samples to the sliding window buffer.

##### Parameters

- **sensor_samples** (*Iterable[float]*): A sequence of numeric values. This can
be a single 3-axis sample `(ax, ay, az)`, multiple concatenated
triples, or any iterable whose flattened length contributes toward
the model’s `input_features_count`.

##### Raises

- **ValueError**: If `sensor_samples` is empty or None.

#### `on_anomaly(callback: callable)`

Register a handler to be invoked when an anomaly is detected.

The callback signature can be one of:
    - `callback()`
    - `callback(anomaly_score: float)`
    - `callback(anomaly_score: float, classification: dict)`

##### Parameters

- **callback** (*callable*): Function to invoke when `anomaly_score >= threshold`.
If a signature with `classification` is used and the model returns
an auxiliary classification head, a dict with label scores is passed.

#### `loop()`

Non-blocking processing step; run this periodically.

Behavior:
    - Pulls a full window from the buffer (if available).
    - Runs inference via `infer_from_features(...)`.
    - Extracts the anomaly score and, if `>= threshold`, invokes the
      registered callback (respecting its signature).

##### Raises

- **StopIteration**: Propagated if an internal shutdown condition is signaled.

#### `start()`

Prepare the detector for a new session.

Notes:
    - Flushes the internal buffer so the next window starts clean.
    - Call before beginning to stream new samples.

#### `stop()`

Stop the detector and release transient resources.

Notes:
    - Clears the internal buffer; does not alter the registered callback.

