# mood_detector API Reference

## Index

- Class `MoodDetector`

---

## `MoodDetector` class

```python
class MoodDetector()
```

A class to detect mood based on text sentiment analysis. It can classify text as **positive**, **negative**, or **neutral**.

Notes:
- Case-insensitive; basic punctuation does not affect results.
- English-only. Non-English or mixed-language input may be treated as neutral.
- Empty or whitespace-only input typically returns neutral.
- Input must be plain text (str).

### Methods

#### `get_sentiment(text: str)`

Analyze the sentiment of the provided text and return the mood.

##### Parameters

- **text** (*str*): The input text to analyze.

##### Returns

- (*str*): The mood of the text — one of `positive`, `negative`, or `neutral`.

