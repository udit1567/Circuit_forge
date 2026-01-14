# audio_classification API Reference

## Index

- Class `AudioClassificationException`
- Class `AudioClassification`

---

## `AudioClassificationException` class

```python
class AudioClassificationException()
```

Custom exception for AudioClassification errors.


---

## `AudioClassification` class

```python
class AudioClassification(mic: Microphone, confidence: float)
```

AudioClassification module for detecting sounds and classifying audio using a specified model.

### Parameters

- **mic** (*Microphone*) (optional): Microphone instance used as the audio source. If None, a default Microphone will be initialized.
- **confidence** (*float*) (optional): Minimum confidence threshold (0.0–1.0) required
for a detection to be considered valid. Defaults to 0.8 (80%).

### Raises

- **ValueError**: If the model information cannot be retrieved, or if model parameters are missing or incomplete.

### Methods

#### `on_detect(class_name: str, callback: Callable[[], None])`

Register a callback function to be invoked when a specific class is detected.

##### Parameters

- **class_name** (*str*): The class to check for in the classification results.
Must match one of the classes defined in the loaded model.
- **callback** (*callable*): Function to execute when the class is detected.
The callback must take no arguments and return None.

##### Raises

- **TypeError**: If `callback` is not callable.
- **ValueError**: If `callback` accepts any argument.

#### `start()`

Start real-time audio classification.

Begins capturing audio from the configured microphone and
continuously classifies the incoming audio stream until stopped.

#### `stop()`

Stop real-time audio classification.

Terminates audio capture and releases any associated resources.

#### `classify_from_file(audio_path: str, confidence: float)`

Classify audio content from a WAV file.

Supported sample widths:
    - 8-bit unsigned
    - 16-bit signed
    - 24-bit signed
    - 32-bit signed

##### Parameters

- **audio_path** (*str*): Path to the `.wav` audio file to classify.
- **confidence** (*float*) (optional): Minimum confidence threshold (0.0–1.0) required
for a detection to be considered valid. Defaults to 0.8 (80%).

##### Returns

-: dict | None: A dictionary with keys:
- ``class_name`` (str): The detected sound class.
- ``confidence`` (float): Confidence score of the detection.
Returns None if no valid classification is found.

##### Raises

- **AudioClassificationException**: If the file cannot be found, read, or processed.
- **ValueError**: If the file uses an unsupported sample width.

