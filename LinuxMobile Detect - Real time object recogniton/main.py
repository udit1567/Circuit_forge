import cv2
import numpy as np

# Set detection threshold and non-max suppression threshold
thres = 0.60  # Confidence threshold to detect objects
nms_threshold = 0.1

# URL of the IP camera or DroidCam app stream
url = 'http://192.168.1.2:4747/video' 
cap = cv2.VideoCapture(url)

# Load class names from the COCO dataset
classNames = []
classFile = "coco.names"
with open(classFile, "r") as f:
    classNames = f.read().rstrip('\n').split('\n')

# Load the pre-trained model's configuration and weights
configPath = 'ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt'
weightsPath = 'frozen_inference_graph.pb'

# Initialize the detection model
net = cv2.dnn_DetectionModel(weightsPath, configPath)
net.setInputSize(320, 320)  # Resize frames to 640x480 for processing
net.setInputScale(1.0 / 127.5)
net.setInputMean((127.5, 127.5, 127.5))
net.setInputSwapRB(True)

# Variables for frame skipping to reduce load
frame_count = 0
skip_frames = 25  # Process every 25th frame as its runnic on phone locally

# Main loop for processing video frames
while True:
    # Read a frame from the video stream
    success, img = cap.read()
    if not success:
        break
    
    frame_count += 1
    if frame_count % (skip_frames + 1) != 0:
        continue

    # Resize the frame for faster processing
    resized_img = cv2.resize(img, (320, 320))

    # Perform object detection on the resized frame
    classIds, confs, bbox = net.detect(resized_img, confThreshold=thres)
    if len(classIds) > 0:
        for classId, confidence, box in zip(classIds.flatten(), confs.flatten(), bbox):
            x, y, w, h = box
            # Map bounding box coordinates back to the original resolution
            x = int(x * (640 / 320))
            y = int(y * (480 / 320))
            w = int(w * (640 / 320))
            h = int(h * (480 / 320))
            # Draw bounding box and label on the original frame
            cv2.rectangle(img, (x, y), (x+w, y+h), color=(0, 255, 0), thickness=2)
            cv2.putText(img, classNames[classId - 1].upper(), (x+10, y+30),
                        cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2)

    # Display the processed frame
    cv2.imshow("Output", img)
    # Exit if 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the video capture object and close OpenCV windows
cap.release()
cv2.destroyAllWindows()