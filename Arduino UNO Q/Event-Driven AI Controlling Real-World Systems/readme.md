# Sense → Act  
### Event-Driven AI Controlling Real-World Systems

---

## Overview

**Sense → Act** is an end-to-end AI system that demonstrates how **computer vision events** can safely and responsibly trigger **real-world physical actions** using an event-driven architecture.

Designed for real-world AI deployments and demonstrations such as the **AI Impact Summit**, this project emphasizes:

- Clear separation between AI perception and hardware control
- Event-driven communication instead of polling
- Safe, explainable, and scalable AI-to-physical-world interaction

---

## Core Idea

> **AI observes.  
> Embedded systems decide.  
> Hardware acts.**

The system follows a strict architectural rule:

- **Python / AI** reports *what it sees*
- **MCU (Arduino)** decides *what to do*
- **MCU alone controls hardware**

This mirrors best practices in safety-critical and embedded AI systems.

---

## System Architecture

```
Camera
  ↓
AI Vision (Python, Linux)
  ↓  (event notification)
RouterBridge
  ↓
Arduino MCU
  ↓
Latch Relay
```

### Roles

| Layer | Responsibility |
|-----|---------------|
| AI / Python | Gesture detection, event notification |
| RouterBridge | Asynchronous message routing |
| MCU (Arduino) | Decision-making and hardware actuation |
| Relay | Physical action |

---

## Features

- 👍 **Thumbs Up** gesture → Relay ON  
- 👎 **Thumbs Down** gesture → Relay OFF  
- Event-driven (no polling, no blocking)
- Debounce-protected gesture handling
- Hardware-safe relay control
- Web UI visualization (optional)

---

## Hardware Requirements

- Arduino-compatible MCU with RouterBridge support (e.g. UNO Q)
- Modulino Latch Relay
- Camera module
- Host system running Linux (for AI inference)

---

## Software Components

### MCU (Arduino)

- Listens for gesture events via RouterBridge
- Maintains internal relay state
- Controls the relay locally

### Python (Linux)

- Runs vision-based gesture detection
- Emits semantic events (`thumbs_up`, `thumbs_down`)
- Never directly controls hardware

---

## Communication Model

### Event Notification (Python → MCU)

```text
Bridge.notify("gesture", "thumbs_up")
Bridge.notify("gesture", "thumbs_down")
```

### Event Handling (MCU)

```text
Bridge.provide("gesture", onGesture)
```

No serial parsing.  
No command strings.  
No tight coupling.

---

## Why This Design Matters

This architecture reflects **responsible AI deployment** principles:

- **Safety**: Hardware logic remains local to the MCU
- **Explainability**: Events are human-readable and traceable
- **Scalability**: New sensors or actions can be added easily
- **Robustness**: AI failures cannot directly damage hardware

This is the same pattern used in industrial automation, robotics, and safety-critical systems.

---

## Example Use Cases

- Touchless control systems
- Assistive technologies
- Smart environments
- AI-controlled safety relays
- Educational demos for embodied AI

---

## Demo Scenario

1. User shows 👍 to the camera  
2. AI detects `thumbs_up`  
3. Event is sent to MCU  
4. MCU turns relay ON  

1. User shows 👎  
2. AI detects `thumbs_down`  
3. Event is sent to MCU  
4. MCU turns relay OFF  

---

## Extensibility

The system can be extended with:

- Gesture confirmation (hold gesture for N seconds)
- Time-based auto-off
- Multi-sensor fusion (vision + gas + temperature)
- Safety interlocks
- Manual override controls

All without changing the core architecture.

---

## Project Philosophy

> **AI should inform the physical world — not directly control it.**

**Sense → Act** demonstrates how to build AI systems that are:
- Responsible
- Maintainable
- Impact-focused
- Ready for real-world deployment



## Acknowledgements

Built using Arduino RouterBridge, Modulino hardware, and event-driven AI principles.

---

**Sense → Act**  
*From perception to responsible action.*
