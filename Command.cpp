#include "Command.h"
#include "Calibration.h"
#include <Arduino.h>

void Command::process(Relay relay[NUM_SOCKETS], Energy energy[NUM_CHANNELS], Sensor &sensor)
{
    if (!Serial.available()) return;

    char cmd = Serial.read();

    // '1', '2', '3' -> toggle that socket's relay (extend NUM_SOCKETS in
    // Config.h and this range follows automatically, up to '9')
    if (cmd >= '1' && cmd < '1' + NUM_SOCKETS)
    {
        int idx = cmd - '1';
        relay[idx].toggle();
        Serial.printf("Socket %d: %s\n", idx + 1, relay[idx].isOn() ? "ON" : "OFF");
        return;
    }

    switch (cmd)
    {
        case 'r':
        case 'R':
            for (int c = 0; c < NUM_CHANNELS; c++) energy[c].reset();
            Serial.println("All energy counters reset");
            break;

        case 'b':
        case 'B':
        {
            TariffBand next = static_cast<TariffBand>((tariff.getBand() + 1) % NUM_BANDS);
            tariff.setBand(next);
            Serial.printf("Tariff band set to Band %s @ N%.2f/kWh (inc. VAT)\n",
                          tariff.getBandName(), tariff.getRate());
            break;
        }

        case 'c':
        case 'C':
        {
            // Flush any leftover bytes (e.g. the '\n' sent right after 'c')
            // so they don't get misread as the voltage input.
            while (Serial.available()) Serial.read();

            Serial.println("Measure mains voltage with a multimeter now.");
            Serial.println("Enter that voltage (e.g. 225.0) and press Enter:");

            unsigned long start = millis();
            while (!Serial.available())
            {
                if (millis() - start > 15000)
                {
                    Serial.println("Timed out waiting for input. Calibration aborted.");
                    return;
                }
            }

            // Give the rest of the line a moment to fully arrive before parsing.
            delay(50);

            float knownVoltage = Serial.parseFloat();

            // Flush trailing newline/garbage so it doesn't leak into the next command read.
            while (Serial.available()) Serial.read();

            if (knownVoltage <= 0.0f)
            {
                Serial.println("Invalid voltage entered. Calibration aborted.");
                break;
            }

            sensor.capture();
            float vOffset = dsp.mean(sensor.voltageBuffer);
            float vrmsADC = dsp.rms(sensor.voltageBuffer, vOffset);

            if (vrmsADC < 1.0f)
            {
                Serial.println("ADC reading too low (mains not connected?). Aborted.");
                break;
            }

            float newScale = knownVoltage / vrmsADC;
            calibration.setVoltScale(newScale);

            Serial.printf("Calibrated. VOLT_SCALE = %.6f (saved to flash)\n", newScale);
            break;
        }
    }
}
