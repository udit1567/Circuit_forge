import cv2
import requests
import numpy as np
from ultralytics import YOLO

# --- Configuration ---
# IMPORTANT: Replace with the IP address of your ESP32-CAM
ESP32_CAM_URL = "http://192.168.1.40" 
# You can change the model here. 'yolov8n.pt' is small and fast.
YOLO_MODEL = 'yolov8l.pt'  

# --- Globals ---
flash_on = False

# Mapping keys to ESP32-CAM framesize enum
# See https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h
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

def put_text_with_shadow(img, text, org, font_face=cv2.FONT_HERSHEY_SIMPLEX, font_scale=0.7, color=(255, 255, 255), thickness=2):
    """Draws text with a black shadow for better visibility."""
    (text_width, text_height), _ = cv2.getTextSize(text, font_face, font_scale, thickness)
    # Draw black shadow
    cv2.putText(img, text, (org[0] + 2, org[1] + 2), font_face, font_scale, (0, 0, 0), thickness, cv2.LINE_AA)
    # Draw white text
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

    # Create a VideoCapture object
    stream_url = f"{ESP32_CAM_URL}/stream"
    cap = cv2.VideoCapture(stream_url)

    if not cap.isOpened():
        print(f"Cannot open stream: {stream_url}")
        print("Please check the ESP32-CAM IP address and that it's streaming.")
        return

    print("Stream opened successfully. Starting detection...")
    print("\n--- Controls ---")
    print("  [1-4] - Change Quality")
    print("  [F]   - Toggle Flash")
    print("  [Q]   - Quit")
    print("----------------\n")

    # Set initial quality
    send_control_command('framesize', current_quality)

    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()
        if not ret or frame is None:
            print("Frame not received, retrying...")
            # Attempt to reconnect
            cap.release()
            cap = cv2.VideoCapture(stream_url)
            cv2.waitKey(1000)
            continue
        
        # Run YOLOv8 inference on the frame
        results = model(frame, verbose=False) # verbose=False to suppress console output

        # Visualize the results on the frame
        annotated_frame = results[0].plot()

        # --- Draw UI elements on the frame ---
        flash_status = "ON" if flash_on else "OFF"
        quality_status = QUALITY_NAMES.get(current_quality, "Unknown")

        # Display instructions and status
        put_text_with_shadow(annotated_frame, "Controls: [1-4] Quality | [F] Flash | [Q] Quit", (10, 30))
        put_text_with_shadow(annotated_frame, f"Quality: {quality_status}", (10, 60))
        put_text_with_shadow(annotated_frame, f"Flash: {flash_status}", (10, 90))

        # Display the resulting frame
        cv2.imshow('ESP32-CAM YOLO Detection', annotated_frame)

        # Handle keyboard input
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('f'):
            flash_on = not flash_on
            send_control_command('flash', 1 if flash_on else 0)
        elif key in QUALITY_MAP:
            new_quality = QUALITY_MAP[key]
            if send_control_command('framesize', new_quality):
                current_quality = new_quality
                # Restart capture to apply new resolution
                print("Resolution changed. Re-initializing video stream...")
                cap.release()
                cap = cv2.VideoCapture(stream_url)

    # When everything done, release the capture
    cap.release()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()
