import serial
import json
import time
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# 🔹 Serial Configuration
SERIAL_PORT = "/dev/cu.wchusbserial110"  # ⚠️ Change this to match your port
BAUD_RATE = 115200
FFT_BANDS = 24  # 🔧 Now matches the number of mel frequency bands
FRAME_INTERVAL = 10  

# 🔹 Mel frequency bands (Adjusted for 16kHz sampling rate)
mel_frequencies = np.array([
    60, 100, 150, 200, 300, 400, 500, 650, 800, 1000, 
    1200, 1400, 1600, 2000, 2400, 2800, 3200, 3600, 4000, 
    4500, 5000, 6000, 7000, 8000
])

# 🔹 Compute bar widths dynamically
bar_widths = np.diff(np.append(mel_frequencies, mel_frequencies[-1] + 500))

# 🔹 Open Serial Connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
time.sleep(2)  # Allow ESP32 to initialize

# 🔹 Initialize Matplotlib
fig, ax_mel = plt.subplots(figsize=(10, 4))
mel_bars = ax_mel.bar(mel_frequencies, np.zeros(FFT_BANDS), width=bar_widths, align="edge", color="royalblue")

ax_mel.set_xscale("log")
ax_mel.set_ylim(0, 120)
ax_mel.set_xlabel("Frequency (Hz)")
ax_mel.set_ylabel("Magnitude (dB)")
ax_mel.set_title("Mel-Scaled Spectrum Analyzer (0-8kHz)")
ax_mel.set_xticks([60, 100, 200, 500, 1000, 2000, 5000, 8000])
ax_mel.get_xaxis().set_major_formatter(plt.ScalarFormatter())

serial_buffer = ""

def update(frame):
    """ Reads serial data, parses it, and updates the visualization. """
    global serial_buffer
    try:
        serial_buffer += ser.read(ser.in_waiting).decode("utf-8", errors="ignore")

        # 🔹 Extract data if a full message is available
        if "BINS:[" in serial_buffer and "]" in serial_buffer:
            start = serial_buffer.find("BINS:[")
            end = serial_buffer.find("]", start) + 1

            json_string = serial_buffer[start:end]
            serial_buffer = serial_buffer[end:]  # Remove processed data

            # 🔹 Parse JSON
            mel_bins = json.loads(json_string[5:])
            
            # 🔧 Fix possible mismatch issue
            if len(mel_bins) != FFT_BANDS:
                print(f"⚠️ Expected {FFT_BANDS} bands but got {len(mel_bins)}, skipping frame.")
                return  

            # 🔹 Update visualization bars
            for i, bar in enumerate(mel_bars):
                bar.set_height(mel_bins[i])

    except json.JSONDecodeError:
        print("⚠️ JSON Decode Error, skipping frame")

# 🔹 Run animation
ani = animation.FuncAnimation(fig, update, interval=FRAME_INTERVAL, cache_frame_data=False)
plt.show()
