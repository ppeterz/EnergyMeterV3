#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---------------- Sockets ----------------

#define NUM_SOCKETS   3
#define NUM_CHANNELS  (NUM_SOCKETS + 1)   // channel 0 = input (total), 1..NUM_SOCKETS = sockets

// ---------------- Pins: sensing ----------------

#define PIN_VOLTAGE         36   // ZMPT101B (shared across all channels)
#define PIN_CURRENT_INPUT   39   // ACS712 - total input current
#define PIN_CURRENT_SOCK1   35   // ACS712 - socket 1
#define PIN_CURRENT_SOCK2   32   // ACS712 - socket 2
#define PIN_CURRENT_SOCK3   33   // ACS712 - socket 3

// ---------------- Pins: relays ----------------

#define PIN_RELAY1   16
#define PIN_RELAY2   17
#define PIN_RELAY3   18

// ---------------- Pins: button + display ----------------

#define PIN_BUTTON   13
#define I2C_SDA      21
#define I2C_SCL      22

// ---------------- ADC ----------------

#define SAMPLE_RATE        4000
#define SAMPLE_INTERVAL_US (1000000 / SAMPLE_RATE)

// At 4000Hz and 50Hz mains, one full cycle = 80 samples exactly.
// 800 samples = 10 whole AC cycles per capture, no fractional-cycle error.
#define BUFFER_SIZE     800

// ---------------- Relay ----------------

#define RELAY_ACTIVE_LOW true

// ---------------- Calibration (fallback defaults if flash is empty) ----------------

#define VOLT_SCALE      0.4290f
#define CURRENT_SCALE   0.0121f

// ---------------- Noise ----------------

#define CURRENT_DEADBAND 0.00f
#define POWER_DEADBAND   3.0f

// ---------------- Firmware & Remote Auto-OTA ----------------

#define FIRMWARE_VERSION      "3.0.3"
#define GITHUB_VERSION_URL    "https://raw.githubusercontent.com/ppeterz/EnergyMeterV3/main/version.txt"
#define GITHUB_FIRMWARE_URL   "https://raw.githubusercontent.com/ppeterz/EnergyMeterV3/main/firmware.bin"

#endif
