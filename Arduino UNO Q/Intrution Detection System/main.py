from arduino.app_utils import App
from arduino.app_bricks.web_ui import WebUI
from arduino.app_bricks.video_objectdetection import VideoObjectDetection
from datetime import datetime, UTC
import threading
import urllib.request
import urllib.parse
import json
import os


BOT_TOKEN = os.getenv("TELEGRAM_BOT_TOKEN")
CHAT_ID = os.getenv("TELEGRAM_CHAT_ID")

if not BOT_TOKEN or not CHAT_ID:
    raise RuntimeError("Telegram BOT_TOKEN or CHAT_ID not set")


ui = WebUI()
detection_stream = VideoObjectDetection(confidence=0.5, debounce_sec=2.0)

# SAFE TELEGRAM TEXT SENDER
def send_text_to_telegram(message):
    try:
        url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"

        data = urllib.parse.urlencode({
            "chat_id": CHAT_ID,
            "text": message
        }).encode("utf-8")

        req = urllib.request.Request(url, data=data, method="POST")
        with urllib.request.urlopen(req, timeout=10) as resp:
            print("[TELEGRAM RESPONSE]", resp.read().decode())

    except Exception as e:
        print("[TELEGRAM ERROR]", repr(e))


# UI threshold override
ui.on_message(
    "override_th",
    lambda sid, threshold: detection_stream.override_threshold(threshold)
)


# Person detected callback
def person_detected(detection):
    confidence = detection.get("confidence", 0.0)

    timestamp = datetime.now(UTC).isoformat()
    message = (
        "🚨 INTRUSION DETECTED 🚨\n"
        f"Object: Person\n"
        f"Confidence: {confidence:.2f}\n"
        f"Time: {timestamp}"
    )

    # Send Telegram asynchronously
    threading.Thread(
        target=send_text_to_telegram,
        args=(message,),
        daemon=True
    ).start()

    # Send UI message
    ui.send_message("detection", message={
        "content": "intrusion detected",
        "confidence": confidence,
        "timestamp": timestamp
    })

    print("[INFO] Intrusion detected (text alert sent)")


# All detections → UI
def send_detections_to_ui(detections: dict):
    for label, details in detections.items():
        ui.send_message("detection", message={
            "content": label,
            "confidence": details.get("confidence"),
            "timestamp": datetime.now(UTC).isoformat()
        })


# Register callbacks
detection_stream.on_detect("person", person_detected)
detection_stream.on_detect_all(send_detections_to_ui)


# Run app
App.run()
