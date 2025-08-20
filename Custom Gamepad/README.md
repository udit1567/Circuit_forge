# 🎮 Customised Gyro Gamepad  

A DIY wireless **gyro-based gamepad** built with an **Arduino Nano**, **MPU6050 sensor**, **joystick**, **buttons**, and **ESP-01 WiFi modules**, communicating with a **Raspberry Pi Pico** as a USB receiver.  

This project enables **motion-controlled gaming** with a response time of ~10ms, providing **real-time orientation + button inputs** for PC games or other applications.  

---

## 📷 Project Overview  

![IMG_2728](https://github.com/user-attachments/assets/59182767-de24-45a4-a98c-2036a4e41005)

The system works as follows:  
1. **Arduino Nano** reads input from:  
   - **MPU6050** → Detects angular motion & orientation.  
   - **Joystick** → Provides X & Y analog control.  
   - **Buttons** → Handle additional input actions.  
2. Data is sent wirelessly via **ESP-01 (Transmitter)**.  
3. **ESP-01 (Receiver)** passes data to a **Raspberry Pi Pico**.  
4. The **Raspberry Pi Pico** processes the data and acts as a **USB HID game controller**.  

---

## 🛠️ Components Used  

- Arduino Nano  
- ESP-01 WiFi Modules (Transmitter + Receiver)  
- MPU6050 (Gyroscope + Accelerometer)  
- Joystick Module  
- Push Buttons (x4)  
- Raspberry Pi Pico  
- Breadboard + Jumper Wires  

---

## 📂 Project Structure  
Custom Gamepad/
│
├── Arduino Nano/
│ └── Arduino.ino # Arduino Nano code (sensor + button handling)
│
├── ESP01-Transmitter/
│ └── esp01-tx.ino # ESP-01 transmitter code
│
├── ESP01-Receiver/
│ └── esp01-rx.ino # ESP-01 receiver code
│
├── Pi-Pico/
│ └── main.py # Raspberry Pi Pico USB-HID receiver code
│
└── README.md # Project documentation

---

## 🚀 How It Works  

- **Step 1:** Arduino Nano collects orientation (gyro), joystick, and button data.  
- **Step 2:** Data is transmitted via **ESP-01** (UART communication).  
- **Step 3:** Another ESP-01 receives the data and forwards it to the **Pi Pico**.  
- **Step 4:** Raspberry Pi Pico interprets the signals and sends them as **HID gamepad inputs** to the PC.  

This creates a **wireless motion + button controller** for gaming or robotics.  

---

## ⚡ Features  

✅ Real-time motion sensing with MPU6050  
✅ Low latency (~10ms response time)  
✅ Joystick + button support  
✅ Wireless data transfer using ESP-01  
✅ Raspberry Pi Pico as USB HID device  
✅ Works with PC games or custom applications  

---

## 🔧 Setup Instructions  

1. Flash `Arduino.ino` to **Arduino Nano**.  
2. Upload `esp01-tx.ino` to the **ESP-01 Transmitter**.  
3. Upload `esp01-rx.ino` to the **ESP-01 Receiver**.  
4. Flash `main.py` to the **Raspberry Pi Pico** (MicroPython).  
5. Connect the **Pi Pico** to your PC → it will show up as a USB gamepad.  
6. Run a game or input tester and enjoy **motion-controlled gaming**!  

---

## 📜 License  

This project is open-source and free to use for educational and personal purposes.  

---



