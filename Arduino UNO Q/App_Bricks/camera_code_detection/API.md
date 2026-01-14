# camera_code_detection API Reference

## Index

- Class `CameraCodeDetection`
- Class `Detection`
- Function `utils.draw_bounding_box`

---

## `CameraCodeDetection` class

```python
class CameraCodeDetection(camera: USBCamera, detect_qr: bool, detect_barcode: bool)
```

Scans a camera video feed for QR codes and/or barcodes.

### Methods

#### `start()`

Start the detector and begin scanning for codes.

#### `stop()`

Stop the detector and release resources.

#### `on_detect(callback: Callable[[Image, list[Detection]], None] | Callable[[Image, Detection], None] | None)`

Registers or removes a callback to be triggered on code detection.

When a QR code or barcode is detected in the camera feed, the provided callback function will be invoked.
The callback function should accept the Image frame and a list[Detection] or Detection objects. If the former
is used, it will receive all detections at once. If the latter is used, it will be called once for each
detection. If None is provided, the callback will be removed.

##### Parameters

- **callback** (*Callable[[Image, list[Detection]], None]*): A callback that will be called every time a detection
is made with all the detections.
- **callback** (*Callable[[Image, Detection], None]*): A callback that will be called every time a detection is
made with a single detection.
- **callback** (*None*): To unregister the current callback, if any.

##### Examples

```python
def on_code_detected(frame: Image, detection: Detection):
    print(f"Detected {detection.type} with content: {detection.content}")
    # Here you can add your code to process the detected code,
    # e.g., draw a bounding box, save it to a database or log it.

detector.on_detect(on_code_detected)
```
#### `on_frame(callback: Callable[[Image], None] | None)`

Registers a callback function to be called when a new camera frame is captured.

The callback function should accept the Image frame.
If None is provided, the callback is removed.

##### Parameters

- **callback** (*Callable[[Image], None]*): A callback that will be called with each captured frame.
- **callback** (*None*): Signals to remove the current callback, if any.

#### `on_error(callback: Callable[[Exception], None] | None)`

Registers a callback function to be called when an error occurs in the detector.

The callback function should accept the exception as an argument.
If None is provided, the callback is removed.

##### Parameters

- **callback** (*Callable*): A callback that will be called with the exception raised in the detector.
- **callback** (*None*): Signals to remove the current callback, if any.

#### `loop()`

Main loop to capture frames and detect codes.


---

## `Detection` class

```python
class Detection(content: str, type: str, coords: np.ndarray)
```

This class represents a single QR code or barcode detection result from a video frame.

This data structure holds the decoded content, the type of code, and its location
in the image as determined by the detection algorithm.

### Attributes

- **content** (*str*): The decoded string extracted from the QR code or barcode.
- **type** (*str*): The type of code detected, typically "QRCODE" or "BARCODE".
- **coords** (*np.ndarray*): A NumPy array of shape (4, 2) representing the four corner
points (x, y) of the detected code region in the image.


---

## `utils.draw_bounding_box` function

```python
def draw_bounding_box(frame: Image, detection: Detection)
```

Draws a bounding box and label on an image for a detected QR code or barcode.

This function overlays a green polygon around the detected code area and
adds a text label above (or below) the bounding box with the code type and content.

### Parameters

- **frame** (*Image*): The PIL Image object to draw on. This image will be modified in-place.
- **detection** (*Detection*): The detection result containing the code's content, type, and corner coordinates.

### Returns

- (*Image*): The annotated image with a bounding box and label drawn.

