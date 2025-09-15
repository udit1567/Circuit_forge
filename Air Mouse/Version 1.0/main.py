import time
import board
import busio
import usb_hid
from adafruit_hid.mouse import Mouse
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode

# Initialize UART for communication with ESP-01
uart = busio.UART(tx=board.GP0, rx=board.GP1, baudrate=115200)

# Initialize Mouse and Keyboard
mouse = Mouse(usb_hid.devices)
keyboard = Keyboard(usb_hid.devices)

while True:
    data = uart.readline()  # Read a line of data from UART
    if data:
        try:
            # Decode the received data and strip any extra whitespace
            data_str = data.decode('utf-8').strip()
            
            # Print the raw received data for tuning
            print(f"Raw Data: {data_str}")

        except UnicodeDecodeError:
            print("Received invalid data, cannot decode.")
    
    # Small delay to prevent excessive output
    time.sleep(0.01)
