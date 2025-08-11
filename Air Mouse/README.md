# Air Mouse using MPU-6050, ESP-01, and Raspberry Pi Pico

![My Projects-37](https://github.com/user-attachments/assets/93c01b87-4746-4f3d-a00b-4ed0db61c337)

## 📌 Overview
This project demonstrates how to create a **wireless Air Mouse** using two distinct modules:
1. **Transmitter Module** — Consists of an MPU-6050 motion sensor, an ESP-01 Wi-Fi module, and two push buttons.
2. **Receiver Module** — Consists of an ESP-01 Wi-Fi module connected to a Raspberry Pi Pico acting as a USB HID mouse.

The MPU-6050 detects **gyroscope and accelerometer data** to control cursor movement, while the push buttons handle left and right clicks. The ESP-01 modules communicate wirelessly via the **ESP-NOW protocol**.

---

## 🛠 Components Required
- **ESP-01 Wi-Fi Module** × 2 (1 transmitter, 1 receiver)
- **MPU-6050** Gyroscope + Accelerometer module
- **Raspberry Pi Pico** (HID mouse emulation)
- **Push Buttons** × 2 (Mouse left & right clicks)
- **Jumper Wires** (Male-to-Male / Female-to-Male)
- **Breadboard** or **Perfboard**
- **USB Cable** for Pico
- **3.3V Power Supply** (for ESP-01 modules)

---

## ⚙️ Wiring

### **Transmitter**
| MPU-6050 Pin | ESP-01 Pin |
|--------------|-----------|
| VCC          | 3.3V      |
| GND          | GND       |
| SCL          | GPIO 0    |
| SDA          | GPIO 2    |

**Push Buttons:**
- Left Click → GPIO 0 with pull-up
- Right Click → GPIO 2 with pull-up

---

### **Receiver**
| ESP-01 Pin | Raspberry Pi Pico Pin |
|------------|-----------------------|
| TX         | GP1 (UART RX)         |
| RX         | GP0 (UART TX)         |
| GND        | GND                   |
| VCC        | 3.3V                  |

---

## 💻 Software Setup

### **1. Program the Transmitter (ESP-01)**
- Install **Arduino IDE**.
- Add **ESP8266 board support** via Board Manager.
- Install required libraries:
  - `ESP8266WiFi`
  - `espnow`
  - `Wire` (for MPU-6050)
- Code tasks:
  1. Initialize MPU-6050 and calibrate it.
  2. Read accelerometer & gyroscope data.
  3. Detect button clicks.
  4. Send all data via **ESP-NOW** to receiver ESP-01.

---

### **2. Program the Receiver (ESP-01)**
- Use Arduino IDE again.
- The receiver ESP-01:
  1. Listens for data from transmitter via ESP-NOW.
  2. Sends the received data over UART to the Raspberry Pi Pico.

---

### **3. Program the Raspberry Pi Pico**
- Flash **MicroPython** or **CircuitPython** firmware.
- Install `adafruit_hid` library.
- Code tasks:
  1. Read data from UART.
  2. Map MPU-6050 data to mouse movement.
  3. Trigger left/right click events from button data.

---

## 🚀 Testing
1. Connect Pico to PC via USB.
2. Move the transmitter — the mouse pointer should move accordingly.
3. Press buttons — left/right clicks should be registered.

---

## 🔧 Optional Improvements
- Apply smoothing algorithms (Kalman/Complementary filters) to stabilize cursor movement.
- Add a LiPo battery for portability.
- Design a 3D-printed case.

---

## ⚠️ Note on Receiver Choice
Using a **Raspberry Pi Pico** as the HID receiver works but **is not the most cost-effective** method.  
You can replace it with an **Arduino Pro Micro** or **Arduino Nano with HID capability**, which can directly act as a USB HID device **at nearly half the cost** of a Pico, reducing hardware complexity.

---
