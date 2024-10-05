import os
import cv2
import numpy as np
import torch
import requests
import threading
import time
from facenet_pytorch import InceptionResnetV1, MTCNN

# Initialize MTCNN and InceptionResnetV1
mtcnn = MTCNN(keep_all=True)
resnet = InceptionResnetV1(pretrained='vggface2').eval()

# Function to detect and encode faces
def detect_and_encode(image):
    with torch.no_grad():
        boxes, _ = mtcnn.detect(image)
        if boxes is not None:
            faces = []
            for box in boxes:
                face = image[int(box[1]):int(box[3]), int(box[0]):int(box[2])]
                if face.size == 0:
                    continue
                face = cv2.resize(face, (160, 160))
                face = np.transpose(face, (2, 0, 1)).astype(np.float32) / 255.0
                face_tensor = torch.tensor(face).unsqueeze(0)
                encoding = resnet(face_tensor).detach().numpy().flatten()
                faces.append((encoding, box))
            return faces
    return []

# Function to encode all images in a folder
def encode_images_in_folder(folder_path):
    known_face_encodings = []
    known_face_names = []

    for filename in os.listdir(folder_path):
        if filename.endswith('.jpg') or filename.endswith('.png'):
            image_path = os.path.join(folder_path, filename)
            known_image = cv2.imread(image_path)
            known_image_rgb = cv2.cvtColor(known_image, cv2.COLOR_BGR2RGB)
            encodings_boxes = detect_and_encode(known_image_rgb)
            if encodings_boxes:
                for encoding, _ in encodings_boxes:
                    known_face_encodings.append(encoding)
                    known_face_names.append(os.path.splitext(filename)[0])  # Use filename as the name

    return known_face_encodings, known_face_names

# Path to your folder containing images
folder_path = 'images' 
known_face_encodings, known_face_names = encode_images_in_folder(folder_path)

# Function to recognize faces
def recognize_faces(known_encodings, known_names, test_encodings, threshold=0.6):
    recognized_names = []
    for test_encoding in test_encodings:
        distances = np.linalg.norm(known_encodings - test_encoding, axis=1)
        min_distance_idx = np.argmin(distances)
        if distances[min_distance_idx] < threshold:
            recognized_names.append(known_names[min_distance_idx])
        else:
            recognized_names.append('Not Recognized')
    return recognized_names

# Function to send unlock request to the ESP32
def send_unlock_request():
    url = 'http://192.168.1.16/unlock'  # Replace with your ESP32 URL (both the phone and esp32 must be connected to same network)
    try:
        response = requests.get(url)
        if response.status_code == 200:
            print('Door unlocked')
        else:
            print('Failed to unlock door')
    except requests.RequestException as e:
        print(f'Request failed: {e}')

# Start video capture from the provided URL
video_url = 'http://192.168.1.6:4747/video'
cap = cv2.VideoCapture(video_url)
threshold = 0.6
frame_skip = 58 # SKipping frames to make processing faster
frame_count = 0
unlock_sent = False

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    frame_count += 1
    if frame_count % frame_skip != 0:
        continue

    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    test_face_encodings_boxes = detect_and_encode(frame_rgb)

    if test_face_encodings_boxes and known_face_encodings:
        test_encodings = [encoding for encoding, _ in test_face_encodings_boxes]
        names = recognize_faces(np.array(known_face_encodings), known_face_names, test_encodings, threshold)

        for (name, (encoding, box)) in zip(names, test_face_encodings_boxes):
            if box is not None:
                (x1, y1, x2, y2) = map(int, box)
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.putText(frame, name, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2, cv2.LINE_AA)

                if name != 'Not Recognized' and not unlock_sent:
                    threading.Thread(target=send_unlock_request).start()
                    unlock_sent = True
                else:
                    unlock_sent = False

    cv2.imshow('Face Recognition', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()