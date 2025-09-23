# ESP32 OLED Studio

A feature-rich web interface hosted on an ESP32 for controlling a 128x64 SSD1306 OLED display in real-time. Upload images, draw, add text, and apply advanced dithering algorithms directly from your browser.

The web app is sent from the ESP32's program memory and communicates with the device via WebSockets, allowing for instant updates on the physical screen.

[Image of the ESP32 OLED Studio web interface]

---

### ## Features

* **Web-Based UI:** No extra software needed. Just connect to the ESP32's IP address in a browser.
* **Image Loading:** Upload images from your computer, via URL, or by simple drag-and-drop.
* **Drawing Tools:** Draw and erase on the canvas with adjustable line widths.
* **Image Transformations:** Invert, rotate 180°, and flip the image horizontally or vertically.
* **Advanced Dithering:** Convert your images to 1-bit monochrome using:
    * **Threshold:** Simple brightness cutoff.
    * **Ordered (Bayer):** Creates a patterned, textured look.
    * **Floyd-Steinberg:** High-quality error diffusion for detailed results.
* **Live Preview:** See the effect of dithering and threshold changes on the canvas before sending.
* **Real-Time Sync:** Uses WebSockets for fast, low-latency communication between the browser and the ESP32.
* **Auto-Send:** Automatically updates the OLED screen with every change you make.

---

### ## Hardware Requirements

* An ESP32 development board.
* A 128x64 SSD1306 I2C OLED Display.
* Jumper Wires.

#### Wiring

Connect the OLED display to your ESP32's default I2C pins.

| OLED Pin | ESP32 Pin |
| :------- | :-------- |
| `VCC`    | `3V3`     |
| `GND`    | `GND`     |
| `SCL`    | `GPIO 22` |
| `SDA`    | `GPIO 21` |

[Image of ESP32 to OLED wiring diagram]

---

### ## Software & Installation

#### 1. Arduino Libraries

Install the following libraries using the Arduino IDE Library Manager (**Sketch > Include Library > Manage Libraries...**):

* `Adafruit GFX Library`
* `Adafruit SSD1306`
* `ESPAsyncWebServer`
* `AsyncTCP`

#### 2. Configure Wi-Fi

Open the `.ino` file and update the following lines with your Wi-Fi network credentials:

```cpp
const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

#### 3. Upload

Connect your ESP32 to your computer, select the correct board and port in the Arduino IDE, and click the upload button.

---

### ## How to Use

1.  **Find the IP Address:** After uploading, open the Serial Monitor (baud rate `115200`). The ESP32 will print its IP address once it connects to your Wi-Fi network.

2.  **Open the Web Interface:** Enter the IP address into a web browser on a device connected to the same network.

3.  **Control the OLED:**
    * Use the controls in the right-hand panels to load an image or draw on the canvas.
    * Adjust dithering settings and see the results in the live preview.
    * If **Auto-send** is enabled, the OLED will update with every change. Otherwise, click **Send to OLED** to update the screen.

---

### ## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
