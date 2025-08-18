
# 📊 Pi Pico Based Oscilloscope



## 📌 Overview
This project turns your **Raspberry Pi Pico** into a **2-channel oscilloscope** using the **Scoppy** app.  
The Pico reads analog signals, processes them, and sends data to your phone or PC for real-time waveform display.

---

## 🛠 Components Required
- **Raspberry Pi Pico** (with micro-USB cable)
- **Resistors**:
  - 2 × 220 Ω
  - 2 × 100 kΩ
- **Breadboard / Jumper wires**
- **Android phone** with **OTG adapter**
- **Scoppy Oscilloscope App** (available on Google Play)

---

## ⚙️ Wiring
Connect components as shown:

| Pico Pin | Component  | Notes         |
|----------|-----------|---------------|
| GP26     | 100 kΩ → CH1 input |
| GP27     | 100 kΩ → CH2 input |
| GND      | 220 Ω → common ground |
| GP28     | 220 Ω → common ground |

📌 See wiring diagram:  
![My Projects-31](https://github.com/user-attachments/assets/54264d81-ca1c-43e3-ae95-5539a8c876f0)
📌 Mobile App:
<img width="468" height="374" alt="image" src="https://github.com/user-attachments/assets/1cd1b63f-0b45-48b7-a8b8-c877ebb38f5d" />


---

## 💻 Software Setup
1. **Download firmware**: `scoppy-pico-v18.uf2`
2. **Flash Pico**:
   - Hold **BOOTSEL** while connecting Pico to PC.
   - Pico appears as a USB drive.
   - Copy `scoppy-pico-v18.uf2` file to Pico.
   - Pico will reboot automatically.
3. **Connect to phone**:
   - Use an **OTG cable** to connect Pico to Android device.
4. **Install & Open App**:
   - Install **Scoppy Oscilloscope** from Google Play.
   - Select **Pico** as device and start capturing.

---

## 📱 How It Works
- **CH1** and **CH2** read analog signals via Pico ADC pins.
- Pico firmware processes and sends data over USB.
- **Scoppy app** displays the real-time waveform, frequency, duty cycle, and voltage.

---

## 🚀 Usage
1. Power Pico via phone OTG or external USB.
2. Open Scoppy App.
3. Adjust **time/div** and **voltage/div** in the app.
4. Connect your test signals to **CH1** or **CH2**.
5. Observe waveforms live.

---

## ⚠️ Notes
- Maximum input voltage is **3.3V** — use a voltage divider for higher voltages.
- For best results, keep cables short to reduce noise.


---

## 📂 Project Structure
<img width="379" height="117" alt="image" src="https://github.com/user-attachments/assets/0be28bb6-a591-4ba1-b223-f658ea3d134a" />



