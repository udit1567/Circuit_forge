import time
import board
import busio
import usb_hid
from adafruit_hid.mouse import Mouse

uart = busio.UART(tx=board.GP0, rx=board.GP1, baudrate=115200)

mouse = Mouse(usb_hid.devices)

prev_button_states = [0, 0]  # Only button1 and button2

def read_uart():
    data = uart.readline()
    if data:
        try:
            # Read button1, button2, acc_x, acc_y
            buttonState1, buttonState2, acc_x, acc_y = [
                float(i) if '.' in i else int(i)
                for i in data.decode('utf-8').strip().split(',')
            ]
            return buttonState1, buttonState2, acc_x, acc_y
        except ValueError:
            pass
    return None, None, None, None

while True:
    buttonState1, buttonState2, acc_x, acc_y = read_uart()

    # Mouse movement
    if acc_x is not None and acc_y is not None:
        mouse.move(int(acc_y * -20), int(acc_x * 12))

    # Button handling (Left click, Right click)
    button_states = [buttonState1, buttonState2]

    for i, (button, prev_button_state, mouse_button) in enumerate(
        zip(button_states, prev_button_states, [Mouse.LEFT_BUTTON, Mouse.RIGHT_BUTTON])
    ):
        if button and not prev_button_state:
            mouse.press(mouse_button)
        elif not button and prev_button_state:
            mouse.release(mouse_button)
        prev_button_states[i] = button

    time.sleep(0.00001)  # Prevent CPU overload
