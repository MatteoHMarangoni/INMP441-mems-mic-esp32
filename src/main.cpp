#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>

// I2S Microphone Pins
#define I2S_WS  16 // 25
#define I2S_SD  17 // 33
#define I2S_SCK 5 // 32

// FFT Parameters
#define SAMPLES 1024
#define SAMPLE_RATE 32000  
#define FFT_BINS (SAMPLES / 2)  
#define FFT_BANDS 24  
#define MIN_MAGNITUDE 1e-6  
#define MIN_DB 0

// FFT Variables
double vReal[SAMPLES];
double vImag[SAMPLES];
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLE_RATE);

// Updated Mel-spaced frequency bands (covering 60Hz - 8kHz)
const float melBands[FFT_BANDS] = {  
    60, 100, 150, 200, 300, 400, 500, 650, 800, 1000, 1200, 1400, 
    1600, 2000, 2400, 2800, 3200, 3600, 4000, 4500, 5000, 6000, 7000, 8000
};

unsigned long lastFFTTime = 0;
const unsigned long FFT_INTERVAL = 10;

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,  //
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = SAMPLES,
        .use_apll = true  
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

bool readI2SData() {
    int16_t rawBuffer[SAMPLES];
    size_t bytesRead;

    i2s_read(I2S_NUM_0, &rawBuffer, sizeof(rawBuffer), &bytesRead, portMAX_DELAY);

    int numSamples = bytesRead / sizeof(int16_t);
    if (numSamples == SAMPLES) {
        for (int i = 0; i < numSamples; i++) {
            vReal[i] = rawBuffer[i];  
            vImag[i] = 0.0;           
        }
        return true;
    }
    return false;
}

void processFFT() {
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();

    int maxValidBin = (8000.0 / (SAMPLE_RATE / SAMPLES));  //
    for (int i = maxValidBin; i < FFT_BINS; i++) {
        vReal[i] = 0;  
    }
}

// ✅ **Fixed: Now sending 24 bands instead of 23**
void sendMelSpectrum() {
    Serial.print("BINS:[");

    for (int i = 0; i < FFT_BANDS; i++) {  //
        int binStart = round(melBands[i] / (SAMPLE_RATE / SAMPLES));
        int binEnd = round(melBands[i + 1] / (SAMPLE_RATE / SAMPLES));

        binStart = constrain(binStart, 0, FFT_BINS - 1);
        binEnd = constrain(binEnd, binStart + 1, FFT_BINS);

        float binSum = 0;
        int binCount = binEnd - binStart;

        for (int j = binStart; j < binEnd; j++) {
            binSum += vReal[j];
        }

        float magnitude = (binCount > 0) ? (binSum / binCount) : 0;
        magnitude = abs(magnitude);
        float magnitudeDB = 20 * log10(max(magnitude, (float)MIN_MAGNITUDE));
        magnitudeDB = constrain(magnitudeDB, MIN_DB, 120);

        Serial.print(magnitudeDB, 2);
        if (i < FFT_BANDS - 1) Serial.print(",");  // 
    }
    Serial.println("]");
}

void setup() {
    Serial.begin(115200);
    setupI2S();
    Serial.println("ESP32 Mel Spectrum Analyzer Ready (0-8kHz).");
}

void loop() {
    if (millis() - lastFFTTime >= FFT_INTERVAL) {
        lastFFTTime = millis();
        if (readI2SData()) {
            processFFT();
            sendMelSpectrum();
        }
    }
}
