from flask import Flask, render_template, Response, jsonify
import cv2, sqlite3, datetime, os
from anpr import ANPR

app = Flask(__name__)
anpr = ANPR(img_size=256)

camera = cv2.VideoCapture("/dev/video0", cv2.CAP_V4L2)

camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
camera.set(cv2.CAP_PROP_FPS, 30)

if not camera.isOpened():
    raise RuntimeError("âŒ Camera not accessible at /dev/video0")

latest_frame = None

DB = "database.db"


def get_db():
    return sqlite3.connect(DB)


def init_db():
    db = get_db()
    db.execute("""
        CREATE TABLE IF NOT EXISTS vehicles (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            plate TEXT UNIQUE,
            entry_time TEXT,
            exit_time TEXT,
            status TEXT
        )
    """)
    db.commit()
    db.close()


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/video_feed")
def video_feed():
    def gen():
        global latest_frame
        while True:
            ret, frame = camera.read()
            if not ret:
                break

            latest_frame = frame.copy()
            _, buffer = cv2.imencode(".jpg", frame)

            yield (b"--frame\r\n"
                   b"Content-Type: image/jpeg\r\n\r\n" +
                   buffer.tobytes() + b"\r\n")

    return Response(gen(), mimetype="multipart/x-mixed-replace; boundary=frame")


@app.route("/capture", methods=["POST"])
def capture():
    global latest_frame

    if latest_frame is None:
        return jsonify([])

    frame, plates = anpr.process_capture(latest_frame.copy())
    os.makedirs("static", exist_ok=True)
    cv2.imwrite("static/captured.jpg", frame)

    db = get_db()
    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    response = []

    for plate in plates:
        cur = db.execute(
            "SELECT status FROM vehicles WHERE plate=?", (plate,))
        row = cur.fetchone()

        if row is None:
            # ENTRY
            db.execute(
                "INSERT INTO vehicles (plate, entry_time, status) VALUES (?, ?, ?)",
                (plate, now, "IN")
            )
            response.append({"plate": plate, "status": "IN", "time": now})

        elif row[0] == "IN":
            # EXIT
            db.execute(
                "UPDATE vehicles SET exit_time=?, status=? WHERE plate=?",
                (now, "OUT", plate)
            )
            response.append({"plate": plate, "status": "OUT", "time": now})

    db.commit()
    db.close()
    return jsonify(response)


@app.route("/vehicles")
def vehicles():
    db = get_db()
    cur = db.execute(
        "SELECT plate, entry_time, exit_time, status FROM vehicles ORDER BY id DESC"
    )
    rows = cur.fetchall()
    db.close()

    return jsonify([
        {
            "plate": r[0],
            "entry_time": r[1],
            "exit_time": r[2],
            "status": r[3]
        } for r in rows
    ])


if __name__ == "__main__":
    init_db()
    app.run(host="0.0.0.0", port=4912, threaded=True)