# SPDX-License-Identifier: MPL-2.0

from arduino.app_utils import App, Bridge
from arduino.app_bricks.web_ui import WebUI
from arduino.app_bricks.video_objectdetection import VideoObjectDetection
from datetime import datetime, UTC

ui = WebUI()

detection_stream = VideoObjectDetection(
    confidence=0.6,
    debounce_sec=1.5
)

# ---------------- Gesture callbacks ----------------
def thumbs_up_detected(detection):
    Bridge.notify("gesture", "thumbs_up")

    ui.send_message("gesture", message={
        "gesture": "thumbs_up",
        "confidence": detection.get("confidence", 0.0),
        "timestamp": datetime.now(UTC).isoformat()
    })


def thumbs_down_detected(detection):
    Bridge.notify("gesture", "thumbs_down")

    ui.send_message("gesture", message={
        "gesture": "thumbs_down",
        "confidence": detection.get("confidence", 0.0),
        "timestamp": datetime.now(UTC).isoformat()
    })

# ---------------- Optional UI feed ----------------
def send_detections_to_ui(detections: dict):
    for label, details in detections.items():
        ui.send_message("detection", message={
            "content": label,
            "confidence": details.get("confidence"),
            "timestamp": datetime.now(UTC).isoformat()
        })

# ---------------- Register callbacks ----------------
detection_stream.on_detect("thumbs_up", thumbs_up_detected)
detection_stream.on_detect("thumbs_down", thumbs_down_detected)
detection_stream.on_detect_all(send_detections_to_ui)

App.run()
