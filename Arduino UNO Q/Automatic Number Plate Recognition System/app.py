import cv2
import torch
import easyocr
from ultralytics import YOLO
from ultralytics.utils.plotting import Annotator, colors


class ANPR:
    def __init__(self, model_path="anpr_best.pt", img_size=640):
        self.model = YOLO(model_path)
        self.reader = easyocr.Reader(["en"], gpu=torch.cuda.is_available())
        self.img_size = img_size

    def process_capture(self, frame):
        results = self.model.predict(
            frame,
            imgsz=self.img_size,
            conf=0.4,
            verbose=False
        )

        annotator = Annotator(frame, line_width=3)
        plates = []

        if results and results[0].boxes is not None:
            boxes = results[0].boxes.xyxy.cpu().numpy()

            for box in boxes:
                x1, y1, x2, y2 = map(int, box)
                roi = frame[y1:y2, x1:x2]

                text = ""
                if roi.size > 0:
                    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
                    ocr = self.reader.readtext(gray, detail=0)
                    if ocr:
                        text = " ".join(ocr).strip()
                        plates.append(text)

                annotator.box_label(
                    box,
                    text if text else "Plate",
                    color=colors(2, True)
                )

        return frame, plates