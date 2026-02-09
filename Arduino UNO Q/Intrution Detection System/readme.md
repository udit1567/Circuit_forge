# AI Intrusion Detection System (Edge + Web UI + Telegram)

End-to-end real-time intrusion detection system using an on-device camera,
Edge AI object detection, Web UI visualization, and Telegram alerts.

<img width="1345" height="737" alt="Screenshot 2026-01-30 at 7 37 57 PM" src="https://github.com/user-attachments/assets/8cb2270e-f87d-4bab-84f9-380c2a8c9ce3" />

This project detects **persons** from a live video stream, displays detections
in a browser-based UI, and sends instant **Telegram alerts** when an intrusion
is detected.



<img width="1280" height="960" alt="image" src="https://github.com/user-attachments/assets/6b1f9dd3-3c6a-4fa9-8eb8-b0e32b345bd7" />

## FEATURES

- Real-time person detection using Edge AI
- Live video inference on-device (no cloud)
- Web UI for live detections and confidence display
- Telegram alert on intrusion detection
- Adjustable confidence threshold from UI
- Debounce protection to avoid alert spam
- Asynchronous alert sending (non-blocking)


## SYSTEM ARCHITECTURE

[CAMERA]
   |
   v
[EDGE AI OBJECT DETECTION]
  - Person detection
  - Confidence filtering
  - Debounce handling
   |
   v
[PYTHON APP RUNTIME]
  - Detection callbacks
  - Telegram alert logic
  - UI messaging
   |
   +---------------------+
   |                     |
   v                     v
[WEB UI DASHBOARD]   [TELEGRAM BOT]
  - Live detections     - Instant alerts
  - Threshold control   - Timestamped messages


## REQUIREMENTS

### Hardware
- Edge device with camera support
- On-device AI acceleration (recommended)

### Software
- Python 3
- Arduino App Runtime
- Web UI brick
- Video Object Detection brick
- Internet access (for Telegram API only)


## ENVIRONMENT VARIABLES

Telegram credentials **must** be set before running the app.

```bash
export TELEGRAM_BOT_TOKEN="your_bot_token_here"
export TELEGRAM_CHAT_ID="your_chat_id_here"
```

If not set, the application will fail at startup.



## APPLICATION COMPONENTS

### Web UI
- Displays detection events
- Shows confidence and timestamps
- Allows runtime threshold override

### VideoObjectDetection
- Runs live object detection on video stream
- Filters detections by confidence
- Debounce interval prevents repeated alerts

Configuration:
- confidence = 0.5
- debounce_sec = 2.0

### Telegram Alert System
- Uses standard Python libraries only
- Sends text alerts asynchronously
- Non-blocking (threaded)


## DETECTION LOGIC

### Person Detection Flow

1) Camera frame processed by AI model
2) Object classified as "person"
3) Confidence checked against threshold
4) Debounce window validated
5) Alert triggered
6) UI updated
7) Telegram message sent


## TELEGRAM ALERT FORMAT

🚨 INTRUSION DETECTED 🚨
Object: Person
Confidence: <value>
Time: <UTC timestamp>


## UI MESSAGES

### Detection Message Payload
```json
{
  "content": "intrusion detected",
  "confidence": 0.87,
  "timestamp": "2026-01-30T10:15:22Z"
}
```

## CALLBACKS OVERVIEW

- `on_detect("person")`
  - Triggers intrusion logic
  - Sends Telegram alert
  - Updates UI

- `on_detect_all`
  - Sends all detections to UI
  - Includes label, confidence, timestamp


## PERFORMANCE NOTES

- Runs fully on-device
- Telegram network calls are asynchronous
- Debounce prevents alert flooding
- Suitable for edge deployment

## USE CASES

- Home security
- Office intrusion detection
- Warehouse monitoring
- Restricted area surveillance
- Edge AI security systems

