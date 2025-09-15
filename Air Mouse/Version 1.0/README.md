
# 🖱 Air Mouse using MPU-6050, ESP-01, and Raspberry Pi Pico
![My Projects-37](https://github.com/user-attachments/assets/93c01b87-4746-4f3d-a00b-4ed0db61c337)

## 📌 Overview
This project creates a **wireless Air Mouse** that controls a computer's mouse pointer by moving a handheld unit.  
It uses:
- **Transmitter Module** → MPU-6050 motion sensor + ESP-01 + two buttons.
- **Receiver Module** → ESP-01 + Raspberry Pi Pico (acting as a USB HID mouse).

The MPU-6050 detects **accelerometer and gyroscope data** to control cursor movement, while the buttons perform **left** and **right clicks**.  
Communication between modules is via **ESP-NOW**.

---

## 🛠 Components Required
- **ESP-01 Wi-Fi Module** × 2 (one for transmitter, one for receiver)
- **MPU-6050** Gyroscope + Accelerometer module
- **Raspberry Pi Pico** (for HID emulation)
- **Push Buttons** × 2 (for left and right clicks)
- **Jumper Wires** (Male-to-Male / Female-to-Male)
- **Breadboard** or **Perfboard**
- **USB Cable** (for Pico)
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
- Left Click → GPIO 0 (pull-up enabled)
- Right Click → GPIO 2 (pull-up enabled)

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
- Open **Arduino IDE**.
- Install **ESP8266 Board Package**.
- Install required libraries:
  - `ESP8266WiFi`
  - `espnow`
  - `Wire` (for I2C)
  - `MPU6050` or similar
- Code should:
  1. Initialize the MPU-6050.
  2. Read gyro & accel data.
  3. Read button states.
  4. Send the data via **ESP-NOW** to the receiver ESP-01.

---

### **2. Program the Receiver (ESP-01)**
- Also programmed using Arduino IDE.
- Code should:
  1. Receive ESP-NOW packets from the transmitter.
  2. Forward the data over UART to the Pico.

---

### **3. Program the Raspberry Pi Pico**
- Flash **MicroPython** or **CircuitPython**.
- Install the `adafruit_hid` library.
- Code should:
  1. Read UART data from the receiver ESP-01.
  2. Convert the data into HID mouse movements and clicks.
  3. Send HID events to the connected PC.

---

## 🚀 How to Run
1. Power on both transmitter and receiver.
2. Connect the Pico to the PC via USB.
3. Move the transmitter — the cursor should follow.
4. Press buttons for left/right clicks.

---

## 🔧 Optional Improvements
- Use a **Kalman filter** to smooth motion data.
- Add a **LiPo battery** for portability.
- Make a **3D-printed enclosure**.

---

## ⚠️ Cost Optimization Note
Using a **Raspberry Pi Pico** as the receiver HID adds **extra hardware cost**.  
You can instead use an **Arduino Pro Micro** (or an Arduino Nano with HID capability), which can directly act as a USB HID device at **about half the cost** of a Pico, simplifying the design.

---

## 📂 Project Structure

<img width="487" height="144" alt="image" src="https://github.com/user-attachments/assets/c6360326-b411-4267-8be3-3ceffbd1f327" />
