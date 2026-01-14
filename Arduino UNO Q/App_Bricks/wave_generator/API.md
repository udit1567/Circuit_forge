# wave_generator API Reference

## Index

- Class `WaveGenerator`

---

## `WaveGenerator` class

```python
class WaveGenerator(sample_rate: int, wave_type: WaveType, block_duration: float, attack: float, release: float, glide: float, speaker: Speaker)
```

Continuous wave generator brick for audio synthesis.

This brick generates continuous audio waveforms (sine, square, sawtooth, triangle)
and streams them to a USB speaker in real-time. It provides smooth transitions
between frequency and amplitude changes using configurable envelope parameters.

The generator runs continuously in a background thread, producing audio blocks
at a steady rate with minimal latency.

### Parameters

- **sample_rate** (*int*): Audio sample rate in Hz (default: 16000).
- **wave_type** (*WaveType*): Initial waveform type (default: "sine").
- **block_duration** (*float*): Duration of each audio block in seconds (default: 0.01).
- **attack** (*float*): Attack time for amplitude envelope in seconds (default: 0.01).
- **release** (*float*): Release time for amplitude envelope in seconds (default: 0.03).
- **glide** (*float*): Frequency glide time (portamento) in seconds (default: 0.02).
- **speaker** (*Speaker*) (optional): Pre-configured Speaker instance. If None, WaveGenerator
will create an internal Speaker optimized for real-time synthesis with:
- periodsize aligned to block_duration (eliminates buffer mismatch)
- queue_maxsize=8 (low latency: ~80ms max buffer)
- format=FLOAT_LE, channels=1

If providing an external Speaker, ensure:
- sample_rate matches WaveGenerator's sample_rate
- periodsize = int(sample_rate × block_duration) for optimal alignment
- Speaker is started/stopped manually (WaveGenerator won't manage its lifecycle)

Example external Speaker configuration:
    speaker = Speaker(
        device="plughw:CARD=UH34",
        sample_rate=16000,
        format="FLOAT_LE",
        periodsize=160,  # 16000 × 0.01 = 160 frames
        queue_maxsize=8
    )

### Raises

- **SpeakerException**: If no USB speaker is found or device is busy.

### Attributes

- **sample_rate** (*int*): Audio sample rate in Hz (default: 16000).
- **wave_type** (*WaveType*): Type of waveform to generate.
- **frequency** (*float*): Current output frequency in Hz.
- **amplitude** (*float*): Current output amplitude (0.0-1.0).

### Methods

#### `start()`

Start the wave generator and audio output.

This starts the speaker device (if internally owned) and launches the producer thread
that continuously generates and streams audio blocks.

#### `stop()`

Stop the wave generator and audio output.

This stops the producer thread and closes the speaker device (if internally owned).

#### `set_frequency(frequency: float)`

Set the target output frequency.

The frequency will smoothly transition to the new value over the
configured glide time.

##### Parameters

- **frequency** (*float*): Target frequency in Hz (typically 20-8000 Hz).

#### `set_amplitude(amplitude: float)`

Set the target output amplitude.

The amplitude will smoothly transition to the new value over the
configured attack/release time.

##### Parameters

- **amplitude** (*float*): Target amplitude in range [0.0, 1.0].

#### `set_wave_type(wave_type: WaveType)`

Change the waveform type.

##### Parameters

- **wave_type** (*WaveType*): One of "sine", "square", "sawtooth", "triangle".

##### Raises

- **ValueError**: If wave_type is not valid.

#### `set_volume(volume: int)`

Set the speaker volume level.

This is a wrapper that controls the hardware volume of the USB speaker device.

##### Parameters

- **volume** (*int*): Hardware volume level (0-100).

##### Raises

- **SpeakerException**: If the mixer is not available or if volume cannot be set.

#### `get_volume()`

Get the current speaker volume level.

##### Returns

- (*int*): Current hardware volume level (0-100).

#### `set_envelope_params(attack: float, release: float, glide: float)`

Update envelope parameters.

##### Parameters

- **attack** (*float*) (optional): Attack time in seconds.
- **release** (*float*) (optional): Release time in seconds.
- **glide** (*float*) (optional): Frequency glide time in seconds.

#### `get_state()`

Get current generator state.

##### Returns

- (*dict*): Dictionary containing current frequency, amplitude, wave type, etc.

