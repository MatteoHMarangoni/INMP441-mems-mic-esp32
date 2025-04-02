// this arduino code for ESP32 (tested WEMOS Lolin D32) reads the data from a mems mic INMP441 over i2s and reports the average amplitude once every second
// audio @ 16bits, 16Khz

/* pin connections
ESP32 3.3V to VDD of INMP441
ESP32 GND to GND and L/R of INMP441
ESP32 GPIO25 to WS of INMP441
ESP32 GPIO33 to SD of INMP441
ESP32 GPIO32 to SCK of INMP441 */


#include <Arduino.h>
#include <driver/i2s.h>

// I2S Microphone Pins
#define I2S_WS  16 // 25
#define I2S_SD  17 // 33
#define I2S_SCK 4 // 32

// Sample rate and buffer size
#define SAMPLE_RATE 16000  // Works well for speech applications
#define BUFFER_SIZE 1024   // Number of samples per read

// Variables for tracking amplitude
uint64_t sumAmplitude = 0;
int sampleCount = 0;
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 1000; // Print every second

void setupI2S() {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S, 
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = BUFFER_SIZE,
      .use_apll = false
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD
  };

  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
      Serial.printf("I2S driver install failed! Error: %d\n", err);
      return;
  } else {
      Serial.println("I2S driver installed successfully.");
  }

  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
      Serial.printf("I2S pin config failed! Error: %d\n", err);
      return;
  } else {
      Serial.println("I2S pin config successful.");
  }

  err = i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
  if (err != ESP_OK) {
      Serial.printf("I2S clock setup failed! Error: %d\n", err);
      return;
  } else {
      Serial.println("I2S clock setup successful.");
  }
}


void readI2SData() {
  int16_t rawBuffer[BUFFER_SIZE]; // Buffer for 16-bit I2S data
  size_t bytesRead;

  esp_err_t result = i2s_read(I2S_NUM_0, &rawBuffer, sizeof(rawBuffer), &bytesRead, 100);

  if (result != ESP_OK || bytesRead == 0) {
      Serial.printf("I2S Read Failed! Error Code: %d, Bytes Read: %d\n", result, bytesRead);
      return;
  }

  int numSamples = bytesRead / sizeof(int16_t);
  //Serial.printf("Bytes Read: %d, Samples: %d\n", bytesRead, numSamples);

  // Process samples and calculate amplitude
  int localSumAmplitude = 0;
  int localSampleCount = 0;

  for (int i = 0; i < numSamples; i++) {
      int16_t sample = rawBuffer[i];
      localSumAmplitude += abs(sample);
      localSampleCount++;
  }

  // Update global variables **only if valid samples exist**
  if (localSampleCount > 0) {
      sumAmplitude += localSumAmplitude;
      sampleCount += localSampleCount;
  }
}



void printAverageAmplitude() {
  if (millis() - lastPrintTime >= printInterval) {
      if (sampleCount > 0) {
          int avgAmplitude = sumAmplitude / sampleCount;
          Serial.printf("📊 Average Amplitude: %d\n", avgAmplitude);

          // Reset counters only if samples were collected
          sumAmplitude = 0;
          sampleCount = 0;
      } else {
          Serial.println("No valid samples collected in the last second.");
      }

      lastPrintTime = millis();
  }
}



void setup() {
    Serial.begin(115200);
    setupI2S();
    Serial.println("INMP441 I2S Microphone Initialized");
}

void loop() {
    readI2SData(); // Continuously read audio samples
    printAverageAmplitude(); // Print amplitude every second
}
