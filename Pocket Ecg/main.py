import smbus
import time
import matplotlib.pyplot as plt
from flask import Flask, render_template, Response
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
import io

# I2C address and command setup
address = 0x48
bus = smbus.SMBus(1)  # Use 1 for Raspberry Pi 2 and later
cmd = 0x40

# Function to read analog data from ADC
def analogRead(chn):
    bus.write_byte(address, cmd + chn)
    value = bus.read_byte(address)
    return value

# Flask web server
app = Flask(__name__)

# Store sensor readings and timestamps
sensor_data = []
time_data = []

# Plotting graph using Matplotlib
@app.route('/plot.png')
def plot_png():
    fig = create_figure()
    output = io.BytesIO()
    FigureCanvas(fig).print_png(output)
    return Response(output.getvalue(), mimetype='image/png')

# Main route to render the webpage
@app.route('/')
def index():
    return render_template('index.html')

# Function to create the plot
def create_figure():
    fig = Figure()
    axis = fig.add_subplot(1, 1, 1)
    axis.set_title("Heart Rate Sensor Readings Over 60 Seconds")
    axis.set_xlabel("Time (s)")
    axis.set_ylabel("Sensor Value")

    # Plot sensor data over time
    axis.plot(time_data, sensor_data, label="Heart Rate")
    axis.set_xlim(left=max(0, time_data[-1] - 10), right=time_data[-1])  # Keep 60s window on x-axis
    axis.legend()

    return fig

# Function to continuously read sensor data
def read_sensor_data():
    global sensor_data, time_data
    start_time = time.time()
    try:
        while True:
            sensor_value = analogRead(1)  # Read from channel 1
            current_time = time.time() - start_time  # Elapsed time in seconds
            sensor_data.append(sensor_value)
            time_data.append(current_time)
            
            # Keep only the last 60 seconds of data
            if time_data[-1] > 60:
                sensor_data = sensor_data[-600:]  # 600 data points (assuming 0.1s delay)
                time_data = time_data[-600:]
            
            print(f"Time: {current_time:.1f}s, Sensor Value: {sensor_value}")
            time.sleep(0.1)  # Delay for stability
    except KeyboardInterrupt:
        pass  # Clean exit on interrupt

if __name__ == "__main__":
    # Start the Flask server in a separate thread
    from threading import Thread
    sensor_thread = Thread(target=read_sensor_data)
    sensor_thread.start()

    # Start Flask web server
    app.run(host="0.0.0.0", port=5000)



