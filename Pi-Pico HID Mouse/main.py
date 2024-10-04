import time
import analogio
import board
import digitalio
import usb_hid
from adafruit_hid.mouse import Mouse
import time
import board
import busio
import adafruit_mpu6050

mouse = Mouse(usb_hid.devices)
i2c = busio.I2C(board.GP1, board.GP0)
mpu = adafruit_mpu6050.MPU6050(i2c)

x_axis = analogio.AnalogIn(board.A0)
y_axis = analogio.AnalogIn(board.A1)

btn_left = digitalio.DigitalInOut(board.GP18)
btn_left.direction = digitalio.Direction.INPUT
btn_left.pull = digitalio.Pull.DOWN

btn_right = digitalio.DigitalInOut(board.GP19)
btn_right.direction = digitalio.Direction.INPUT
btn_right.pull = digitalio.Pull.DOWN

pot_min = 0.00
pot_max = 3.29
step = (pot_max - pot_min) / 20.0


def get_voltage(pin):
    return (pin.value * 3.3) / 65536


def steps(axis):

    return round((axis - pot_min) / step)

def map_range(value, in_min, in_max, out_min, out_max):
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def map_accelerationy(value):
    return map_range(value, -10, 10, -8, 8)
def map_accelerationx(value):
    return map_range(value, 10, -10, -8, 8)


while True:
    acc_x, acc_y, _ = mpu.acceleration

    x = map_accelerationy(acc_y)
    y = map_accelerationx(acc_x)

    if btn_left.value:
        mouse.click(Mouse.LEFT_BUTTON)
        time.sleep(0.2)
    if btn_right.value:
        mouse.click(Mouse.RIGHT_BUTTON)
        time.sleep(0.2)

    mouse.move(x=int(x), y=int(y))
