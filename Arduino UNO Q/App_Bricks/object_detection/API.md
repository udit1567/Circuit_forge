# object_detection API Reference

## Index

- Class `ObjectDetection`

---

## `ObjectDetection` class

```python
class ObjectDetection(confidence: float)
```

Module for object detection in images using a specified machine learning model.

This module processes an input image and returns:
- Bounding boxes for detected objects
- Corresponding class labels
- Confidence scores for each detection

### Parameters

- **confidence** (*float*): Minimum confidence threshold for detections. Default is 0.3 (30%).

### Raises

- **ValueError**: If model information cannot be retrieved.

### Methods

#### `detect_from_file(image_path: str, confidence: float)`

Process a local image file to detect and identify objects.

##### Parameters

- **image_path**: Path to the image file on the local file system.
- **confidence**: Minimum confidence threshold for detections. Default is None (use module defaults).

##### Returns

- (*dict*): Detection results containing class names, confidence, and bounding boxes.

#### `detect(image_bytes, image_type: str, confidence: float)`

Process an in-memory image to detect and identify objects.

##### Parameters

- **image_bytes**: Can be raw bytes (e.g., from a file or stream) or a preloaded PIL image.
- **image_type**, default='jpg': The image format ('jpg', 'jpeg', or 'png'). Required if using raw bytes. Defaults to 'jpg'.
- **confidence**: Minimum confidence threshold for detections. Default is None (use module defaults).

##### Returns

- (*dict*): Detection results containing class names, confidence, and bounding boxes.

#### `draw_bounding_boxes(image: Image.Image | bytes, detections: dict)`

Draw bounding boxes on an image enclosing detected objects using PIL.

##### Parameters

- **image**: The input image to annotate. Can be a PIL Image object or raw image bytes.
- **detections**: Detection results containing object labels and bounding boxes.

##### Returns

-: Image with bounding boxes and key points drawn.
None if input image or detections are invalid.

#### `process(item)`

Process an item to detect objects in an image.

This method supports two input formats:
- A string path to a local image file.
- A dictionary containing raw image bytes under the 'image' key, and optionally an 'image_type' key (e.g., 'jpg', 'png').

##### Parameters

- **item**: A file path (str) or a dictionary with the 'image' and 'image_type' keys (dict).
'image_type' is optional while 'image' contains image as bytes.

