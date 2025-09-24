# 🖱️ Air Mouse (Version 2.0)

This project transforms an **ESP32** and an **MPU6050 (Gyroscope + Accelerometer)** into a **wireless Air Mouse** using **Bluetooth Low Energy (BLE)**.  
By moving the ESP32 device in the air, the cursor on your computer moves accordingly. Additional buttons are provided for **left click, right click, and scroll** functionality.  

---

## 📷 Circuit Diagram

![IMG_2962 (2)](https://github.com/user-attachments/assets/85f1796a-e780-4030-9029-ef5a011791b4)



---

## ⚡ Features

- Use your **ESP32 as a BLE Mouse**.
- Motion-controlled cursor using **MPU6050 gyroscope**.
- Four physical buttons:
  - **Left Click**
  - **Right Click**
  - **Scroll Up**
  - **Scroll Down**
- Built-in **gyro calibration** for stable control.
- **Moving average filter** to reduce jitter.
- Adjustable **sensitivity and deadzone**.

---

## 🛠️ Components Required

- **ESP32 Dev Module (ESP-WROOM-32)**
- **MPU6050 (Gyroscope + Accelerometer)**
- **4 Push Buttons**
- Jumper wires
- Breadboard or PCB

---

## 🔌 Pin Connections

| ESP32 Pin | MPU6050 Pin | Function         |
|-----------|-------------|------------------|
| 3V3       | VCC         | Power            |
| GND       | GND         | Ground           |
| 22        | SCL         | I²C Clock        |
| 21        | SDA         | I²C Data         |

| ESP32 Pin | Button      | Function         |
|-----------|-------------|------------------|
| 12        | Button 1    | Left Click       |
| 13        | Button 2    | Right Click      |
| 14        | Button 3    | Scroll Up        |
| 27        | Button 4    | Scroll Down      |

---

## 📦 Arduino Libraries Required

Install the following libraries in Arduino IDE:

1. **BleMouse**  
   ```arduino
   Sketch → Include Library → Manage Libraries → Search "BleMouse"

<img width="747" height="592" alt="image" src="https://github.com/user-attachments/assets/b23ebeca-0c2c-4db6-bfd6-6db748585c25" />

