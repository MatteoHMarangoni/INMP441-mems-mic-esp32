// this arduino code for ESP32 (tested WEMOS Lolin D32) reads the data from a mems mic INMP441 over i2s and reports the average amplitude once every second
// audio @ 16bits, 16Khz

/* pin connections
ESP32 3.3V to VDD of INMP441
ESP32 GND to GND and L/R of INMP441
ESP32 GPIO25 to WS of INMP441
ESP32 GPIO33 to SD of INMP441
ESP32 GPIO32 to SCK of INMP441 */
