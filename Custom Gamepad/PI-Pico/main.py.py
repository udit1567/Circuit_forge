import time
import board
import busio
import usb_hid
from adafruit_hid.mouse import Mouse
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode

uart = busio.UART(tx=board.GP0, rx=board.GP1, baudrate=115200)

mouse = Mouse(usb_hid.devices)
keyboard = Keyboard(usb_hid.devices)

prev_button_states = [0, 0, 0, 0]
prev_joyX = None
prev_joyY = None

def read_uart():
    data = uart.readline()
    if data:
        try:
            # Parse UART data
            joyX, joyY, buttonState1, buttonState2, buttonState3, buttonState4, acc_x, acc_y = [float(i) if '.' in i else int(i) for i in data.decode('utf-8').strip().split(',')]
            return joyX, joyY, buttonState1, buttonState2, buttonState3, buttonState4, acc_x, acc_y
        except ValueError:
            pass
    return None, None, None, None, None, None, None, None

while True:
    joyX, joyY, buttonState1, buttonState2, buttonState3, buttonState4, acc_x, acc_y = read_uart()

    # Mouse movement
    if acc_x is not None and acc_y is not None:
        mouse.move(int(acc_y * -20), int(acc_x * 12))

    # Joystick handling (W, A, S, D)
    if joyY is not None and joyX is not None:
        if joyY < 400 and prev_joyY != "forward":
            keyboard.press(Keycode.W)
            prev_joyY = "forward"
        elif joyY >= 400 and prev_joyY == "forward":
            keyboard.release(Keycode.W)
            prev_joyY = None

        if joyY > 600 and prev_joyY != "backward":
            keyboard.press(Keycode.S)
            prev_joyY = "backward"
        elif joyY <= 600 and prev_joyY == "backward":
            keyboard.release(Keycode.S)
            prev_joyY = None

        if joyX > 600 and prev_joyX != "left":
            keyboard.press(Keycode.A)
            prev_joyX = "left"
        elif joyX <= 600 and prev_joyX == "left":
            keyboard.release(Keycode.A)
            prev_joyX = None

        if joyX < 450 and prev_joyX != "right":
            keyboard.press(Keycode.D)
            prev_joyX = "right"
        elif joyX >= 450 and prev_joyX == "right":
            keyboard.release(Keycode.D)
            prev_joyX = None

    # Button handling (F, SHIFT, H, J)
    button_states = [buttonState1, buttonState2, buttonState3, buttonState4]

    for i, (button, prev_button_state, keycode) in enumerate(zip(button_states, prev_button_states, [Keycode.F, Keycode.SHIFT, Keycode.H, Keycode.J])):
        if button and not prev_button_state:
            keyboard.press(keycode)
        elif not button and prev_button_state:
            keyboard.release(keycode)
        prev_button_states[i] = button

    time.sleep(0.00001)  # Minimal sleep to prevent CPU overload
