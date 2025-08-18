# 🕷️ ESP32-CAM Based Spider Bot  

This project is an **advanced quadruped robot (Spider Bot)** that uses an **Arduino Nano** to control 12 servo motors for leg movement, while an **ESP32-CAM** module is integrated for live video streaming and remote control.  

The idea is inspired by an open-source quadruped design, but extended further by adding **wireless video streaming and web-based control**, making it more interactive and versatile.

---

## 📌 Features  
- **12 Servo Motors** – Three per leg, enabling realistic spider-like walking and movement.  
- **Arduino Nano** – Handles servo motor control and movement sequences.  
- **ESP32-CAM** – Provides a live video feed for first-person (FPV) view and acts as a Wi-Fi-based controller.  
- **Remote Control** – Control the bot’s movements wirelessly from a smartphone or PC.  
- **Live Streaming** – Real-time video directly from the ESP32-CAM.  
- **Expandable** – Can be upgraded with additional sensors like ultrasonic or IMU for autonomous navigation.  

---

## 📂 Project Structure  
<img width="661" height="186" alt="image" src="https://github.com/user-attachments/assets/58627bb8-ab27-421a-9ec7-d1f8c8d4d81e" />

---

## ⚡ Circuit Diagram  
Below is the connection diagram showing how the Arduino Nano, ESP32-CAM, and servo motors are connected:  

![a](https://github.com/user-attachments/assets/1cd4a6c5-4839-4ef3-b440-75298313a785)

- **Arduino Nano** controls 12 servo motors.  
- **ESP32-CAM** handles live video streaming and sends commands to Arduino.  
- Both boards share a **common power supply**.


---

## 🛠️ Components Required  
- Arduino Nano V3  
- ESP32-CAM module  
- 12x Servo Motors (SG90 or MG90S recommended)  
- Power Supply (5V 3A minimum for stable servo operation)  
- Jumper wires and connectors  
- 3D-printed frame and legs (STL files included)  

---

## ⚙️ Working  
1. The **Arduino Nano** runs predefined movement sequences (walk forward, backward, turn, stand, sit).  
2. The **ESP32-CAM** streams real-time video and provides a control interface.  
3. Commands are sent from the ESP32-CAM (via UART) to the Arduino Nano to execute specific movement functions.  
4. User connects to the ESP32-CAM Wi-Fi hotspot, opens the control page, and can **view + control the Spider Bot**.  

---

## 🚀 How to Run  
1. Flash the **Arduino Nano** with the provided servo control code.  
2. Flash the **ESP32-CAM** with `ESP-32.ino`.  
3. Power both modules from a **regulated 5V source** (ensure enough current for all servos).  
4. Connect to the ESP32-CAM Wi-Fi and open the control interface in a browser.  
5. Use the control buttons to move the robot while watching the live video feed.  

---

## 📸 Demo / Preview  
![IMG000](https://github.com/user-attachments/assets/c6a7d8a1-bb3c-424e-b021-fa1d5abe038f)


---

## 🔗 Reference & Inspiration  
This project is based on an open-source quadruped design:  
👉 [DIY Spider Robot (Quadruped)](https://www.instructables.com/DIY-Spider-RobotQuad-robot-Quadruped/)  

I have **leveled it up** by integrating an **ESP32-CAM for live video streaming and remote web control**, making the robot more interactive and IoT-enabled.  

---
![Screenshot 2025-01-06 174728](https://github.com/user-attachments/assets/f58d69cf-daf6-47a0-9d2c-cdde48475625)
![Screenshot 2025-01-06 131307](https://github.com/user-attachments/assets/db954234-540c-45f9-aa33-be22acfb9510)
![Screenshot 2025-01-06 160735](https://github.com/user-attachments/assets/619a896b-4e78-4afb-bea5-d79f8ffa2c68)
![Screenshot 2025-01-06 160753](https://github.com/user-attachments/assets/eba56e55-8d2c-40dc-9cbd-0abf6f76d71a)



