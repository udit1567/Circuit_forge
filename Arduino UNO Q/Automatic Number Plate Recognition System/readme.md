# Automatic Number Plate Recognition (ANPR) System 🚗📷

End-to-end Automatic Number Plate Recognition (ANPR) system using
Flask, OpenCV, YOLO, EasyOCR, and SQLite.

This project captures live video from a camera, detects vehicle number plates
using a deep learning model, extracts plate text using OCR, and automatically
logs vehicle entry and exit events in a database with a web-based dashboard.


<img width="513" height="736" alt="Screenshot 2026-01-30 at 7 30 03 PM" src="https://github.com/user-attachments/assets/4ce95473-91d6-41de-bfd2-f204f48a9c11" />


## FEATURES

- Live camera video streaming (MJPEG)
- Automatic number plate detection (YOLO)
- Optical Character Recognition (EasyOCR)
- Vehicle entry / exit detection
- SQLite database for vehicle logs
- Web-based dashboard (Flask)
- REST APIs for vehicle history
- Real-time annotated frame capture


## SYSTEM ARCHITECTURE

[CAMERA (/dev/video0)]
        |
        v
[OPENCV VIDEO CAPTURE]
        |
        v
[YOLO ANPR MODEL]
  - Plate detection
        |
        v
[EASYOCR]
  - Text extraction
        |
        v
[ANPR LOGIC]
  - Entry / Exit decision
        |
        v
[SQLITE DATABASE]
        |
        +----------------------+
        |                      |
        v                      v
[FLASK WEB UI]         [REST API ENDPOINTS]
  - Live video           - Vehicle logs
  - Capture button


## REQUIREMENTS

### Hardware
- USB camera or CSI camera
- Linux system (tested with /dev/video0)

### Software
- Python 3.8+
- OpenCV
- Flask
- PyTorch
- Ultralytics YOLO
- EasyOCR
- SQLite3


## INSTALLATION

### 1) Create Virtual Environment (Recommended)

```bash
python3 -m venv venv
source venv/bin/activate
```

### 2) Install Dependencies

```bash
pip install flask opencv-python torch easyocr ultralytics
```

(Optional GPU support requires CUDA-enabled PyTorch)


## CAMERA CONFIGURATION

Camera device:
- `/dev/video0`

Settings:
- Resolution: 640 × 480
- FPS: 30
- Backend: V4L2

If camera is not accessible, the app will fail with:
```
Camera not accessible at /dev/video0
```


## DATABASE DESIGN

Database file:
- `database.db`

Table: `vehicles`

Columns:
- id (INTEGER, PRIMARY KEY)
- plate (TEXT, UNIQUE)
- entry_time (TEXT)
- exit_time (TEXT)
- status (TEXT)

Status values:
- IN  → vehicle entered
- OUT → vehicle exited


## APPLICATION FLOW

1) Live video stream starts
2) User clicks "Capture"
3) Current frame is processed
4) YOLO detects number plate
5) EasyOCR extracts text
6) Plate checked in database
7) Entry or Exit is recorded
8) Annotated frame is saved
9) UI + API updated


## FLASK ROUTES

### `/`
- Main dashboard page

### `/video_feed`
- Live MJPEG video stream

### `/capture`  (POST)
- Captures current frame
- Runs ANPR
- Updates database
- Returns JSON result

Example Response:
```json
[
  {
    "plate": "GJ01AB1234",
    "status": "IN",
    "time": "2026-01-30 12:30:15"
  }
]
```

### `/vehicles`
- Returns all vehicle records

Example Response:
```json
[
  {
    "plate": "GJ01AB1234",
    "entry_time": "2026-01-30 12:30:15",
    "exit_time": null,
    "status": "IN"
  }
]
```

## ANPR MODULE DETAILS

### Model
- YOLO (Ultralytics)
- Model file: `anpr_best.pt`

### OCR
- EasyOCR (English)
- GPU acceleration enabled if available

### Detection Parameters
- Image size: 256 / 640
- Confidence threshold: 0.4

## ENTRY / EXIT LOGIC

- If plate not in database → ENTRY
- If plate exists and status = IN → EXIT
- Duplicate entries prevented using UNIQUE plate constraint


## PERFORMANCE NOTES

- Real-time inference depends on hardware
- GPU significantly improves OCR performance
- Suitable for edge devices and local servers


## LIMITATIONS

- OCR accuracy depends on lighting and plate quality
- Single camera supported (extendable)
- No authentication (demo-focused)

## USE CASES

- Parking management systems
- Gated communities
- Toll booths
- Campus vehicle monitoring
- Smart city traffic systems
