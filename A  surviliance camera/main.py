import cv2
from ultralytics import YOLO
import sys

def main():
    """
    Main function to capture video from a stream and perform YOLOv10 object detection.
    """
    # --- Configuration ---
    # The URL of the video stream.
    # Replace this with your video stream's URL.
    VIDEO_STREAM_URL = "http://192.168.1.40:81/stream"

    # Load the YOLOv10 model.
    # 'yolov10n.pt' is the nano version, which is fast and lightweight.
    # For higher accuracy, you can use 'yolov10s.pt', 'yolov10m.pt', etc.
    # The model will be downloaded automatically on the first run.
    try:
        model = YOLO('yolov10m.pt')
    except Exception as e:
        print(f"Error loading YOLO model: {e}")
        print("Please ensure you have an internet connection to download the model.")
        sys.exit(1)


    # Open the video stream
    cap = cv2.VideoCapture(VIDEO_STREAM_URL)

    # Check if the video stream was opened successfully
    if not cap.isOpened():
        print(f"Error: Could not open video stream at {VIDEO_STREAM_URL}")
        print("Please check the URL and ensure the stream is active.")
        sys.exit(1)
    else:
        print("Successfully connected to the video stream.")
        print("Press 'q' to quit.")

    # --- Main Loop ---
    while True:
        # Read a frame from the stream
        ret, frame = cap.read()

        # If the frame was not read successfully, break the loop
        if not ret:
            print("Failed to grab frame from the stream. Stream might have ended.")
            break

        # Perform object detection on the frame
        # Using stream=True is more memory-efficient for video processing
        results = model(frame, stream=True)

        # Process results
        for r in results:
            # The .plot() method automatically draws bounding boxes and labels on the frame
            annotated_frame = r.plot()

            # Display the annotated frame in a window
            cv2.imshow("YOLOv10 Object Detection", annotated_frame)

        # Check for user input to quit
        # Wait for 1 millisecond and check if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # --- Cleanup ---
    # Release the video capture object
    cap.release()
    # Destroy all OpenCV windows
    cv2.destroyAllWindows()
    print("Stream stopped and windows closed.")

if __name__ == "__main__":
    main()
