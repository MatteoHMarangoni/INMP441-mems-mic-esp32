ESP32 Mel Spectrum Analyzer (0-8kHz)
A real-time Mel-scaled spectrum analyzer using an ESP32 and an INMP441 I2S microphone. The ESP32 performs FFT (Fast Fourier Transform) to analyze audio in the 0-8kHz range and sends 24 mel-scaled frequency bands to a Python visualizer.


/* pin connections
ESP32 3.3V to VDD of INMP441
ESP32 GND to GND and L/R of INMP441
ESP32 GPIO16 to WS of INMP441
ESP32 GPIO17 to SD of INMP441
ESP32 GPIO05 to SCK of INMP441 */

For the visualiser: 

Ensure you have Python 3.8+, then install dependencies:
pip install pyserial numpy matplotlib

Run the Visualizer
Change SERIAL_PORT to your ESP32's port (e.g., /dev/cu.wchusbserial110 for macOS/Linux or COM3 for Windows), then:
python3 spectrum_analyser.py

note: the amplitude indication in decibels are purely an indication, the visualiser is not calibrated