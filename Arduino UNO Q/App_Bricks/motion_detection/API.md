# motion_detection API Reference

## Index

- Class `MotionDetection`

---

## `MotionDetection` class

```python
class MotionDetection(confidence: float)
```

This Motion Detection module classifies motion patterns using accelerometer data.

### Parameters

- **confidence** (*float*): Confidence level for detection. Default is 0.4 (40%).

### Methods

#### `on_movement_detection(movement: str, callback: callable)`

Register a callback function to be invoked when a specific motion pattern is detected.

##### Parameters

- **movement** (*str*): The motion pattern name to check for in the classification results.
- **callback** (*callable*): Function to call when the specified motion pattern is detected.

#### `accumulate_samples(accelerometer_samples: Tuple[float, float, float])`

Accumulate accelerometer samples for motion detection.

##### Parameters

- **accelerometer_samples** (*tuple*): A tuple containing x, y, z acceleration values. Typically, these values are
in m/s^2, but depends on the model configuration.

#### `get_sensor_samples()`

Get the current sensor samples.

##### Returns

- (*iterable*): An iterable containing the accumulated sensor data (x, y, z acceleration values).

