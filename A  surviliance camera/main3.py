import cv2
import requests
import numpy as np
import urllib.parse
import threading
import time
from ultralytics import YOLO

# --- Configuration ---
# IMPORTANT: Replace with the IP address of your ESP32-CAM
ESP32_CAM_URL = "http://192.168.1.40"
# You can change the model here. 'yolov8n.pt' is small and fast.
YOLO_MODEL = 'yolov8l.pt'
# Confidence threshold (0.0 to 1.0). Detections below this will be ignored.
CONFIDENCE_THRESHOLD = 0.5

# --- Globals ---
flash_on = False
last_sent_name = ""
last_oled_error_time = 0

# Mapping keys to ESP32-CAM framesize enum
QUALITY_MAP = {
    ord('1'): 8,  # VGA, 640x480
    ord('2'): 6,  # CIF, 400x296
    ord('3'): 5,  # QVGA, 320x240
    ord('4'): 3,  # QQVGA, 160x120
}
QUALITY_NAMES = {
    8: "VGA (640x480)",
    6: "CIF (400x296)",
    5: "QVGA (320x240)",
    3: "QQVGA (160x120)",
}
current_quality = 8 # Default to VGA

# --- Thread-Safe Video Stream Class for Lag Reduction ---
class VideoStream:
    """A class to read frames from a camera stream in a separate thread."""
    def __init__(self, src=0):
        self.stream = cv2.VideoCapture(src)
        (self.grabbed, self.frame) = self.stream.read()
        self.stopped = False

    def start(self):
        threading.Thread(target=self.update, args=()).start()
        return self

    def update(self):
        while True:
            if self.stopped:
                self.stream.release()
                return
            (self.grabbed, self.frame) = self.stream.read()

    def read(self):
        return self.frame

    def stop(self):
        self.stopped = True


def send_control_command(variable, value):
    """Sends a GET request to the ESP32-CAM to change a setting."""
    url = f"{ESP32_CAM_URL}/control?var={variable}&val={value}"
    try:
        response = requests.get(url, timeout=5)
        if response.status_code == 200:
            print(f"Successfully set {variable} to {value}")
            return True
        else:
            print(f"Failed to set {variable}. Status: {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"Error sending command: {e}")
        return False

def send_display_text(text):
    """Sends text to be displayed on the ESP32's OLED."""
    global last_sent_name, last_oled_error_time
    if text == last_sent_name:
        return  # Don't send if the text hasn't changed

    encoded_text = urllib.parse.quote(text)
    url = f"{ESP32_CAM_URL}/update-display?text={encoded_text}"
    try:
        requests.get(url, timeout=1)
        print(f"Sent '{text}' to display.")
        last_sent_name = text
    except requests.exceptions.RequestException as e:
        # Print error only once every 5 seconds to avoid spamming the console
        current_time = time.time()
        if current_time - last_oled_error_time > 5:
            print(f"OLED Update Error: {e}")
            last_oled_error_time = current_time

def put_text_with_shadow(img, text, org, font_face=cv2.FONT_HERSHEY_SIMPLEX, font_scale=0.7, color=(255, 255, 255), thickness=2):
    """Draws text with a black shadow for better visibility."""
    cv2.putText(img, text, (org[0] + 2, org[1] + 2), font_face, font_scale, (0, 0, 0), thickness, cv2.LINE_AA)
    cv2.putText(img, text, org, font_face, font_scale, color, thickness, cv2.LINE_AA)

def main():
    global flash_on, current_quality
    
    # Load the YOLO model
    try:
        model = YOLO(YOLO_MODEL)
    except Exception as e:
        print(f"Error loading YOLO model: {e}")
        print("Please make sure you have installed ultralytics: pip install ultralytics")
        return

    # Start the threaded video stream
    stream_url = f"{ESP32_CAM_URL}/stream"
    vs = VideoStream(stream_url).start()
    time.sleep(2.0) # Allow camera sensor to warm up

    print("Stream opened successfully. Starting detection...")
    print("\n--- Controls ---")
    print("  [1-4] - Change Quality")
    print("  [F]   - Toggle Flash")
    print("  [Q]   - Quit")
    print("----------------\n")

    # Set initial quality
    send_control_command('framesize', current_quality)

    while True:
        frame = vs.read()
        if frame is None:
            print("Frame not received, waiting...")
            time.sleep(0.5)
            continue
        
        # Run YOLOv8 inference
        results = model(frame, verbose=False, conf=CONFIDENCE_THRESHOLD)

        # Visualize the results
        annotated_frame = results[0].plot()
        
        # --- Get detected object name and send to ESP32 ---
        detected_object_name = "..." # Default text
        if results[0].boxes.cls.numel() > 0:
            top_class_id = int(results[0].boxes.cls[0].item())
            detected_object_name = results[0].names[top_class_id]
        
        send_display_text(detected_object_name)

        # --- Draw UI elements on the frame ---
        flash_status = "ON" if flash_on else "OFF"
        quality_status = QUALITY_NAMES.get(current_quality, "Unknown")

        put_text_with_shadow(annotated_frame, "Controls: [1-4] Quality | [F] Flash | [Q] Quit", (10, 30))
        put_text_with_shadow(annotated_frame, f"Quality: {quality_status}", (10, 60))
        put_text_with_shadow(annotated_frame, f"Flash: {flash_status}", (10, 90))

        cv2.imshow('ESP32-CAM YOLO Detection', annotated_frame)

        # Handle keyboard input
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            send_display_text("Offline")
            break
        elif key == ord('f'):
            flash_on = not flash_on
            send_control_command('flash', 1 if flash_on else 0)
        elif key in QUALITY_MAP:
            new_quality = QUALITY_MAP[key]
            if send_control_command('framesize', new_quality):
                current_quality = new_quality
                print("Resolution changed. You may need to restart the script for the window size to update correctly.")

    vs.stop()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()

