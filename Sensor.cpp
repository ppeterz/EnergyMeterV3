#include "Sensor.h"

static const int currentPins[NUM_CHANNELS] = {
    PIN_CURRENT_INPUT, PIN_CURRENT_SOCK1, PIN_CURRENT_SOCK2, PIN_CURRENT_SOCK3
};

void Sensor::begin()
{
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_VOLTAGE, ADC_11db);
    for (int c = 0; c < NUM_CHANNELS; c++)
        analogSetPinAttenuation(currentPins[c], ADC_11db);
}

void Sensor::capture()
{
    // Every tick reads voltage AND all 4 currents together, so they stay
    // time-aligned for real-power calculation on every channel -- same
    // principle as the single-channel version, just extended to 4 currents.
    // This keeps total capture time the same as before (BUFFER_SIZE x
    // SAMPLE_INTERVAL_US, ~200ms) regardless of channel count, since all
    // channels are read within the same per-sample window rather than as
    // separate sequential captures.
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        unsigned long t = micros();

        voltageBuffer[i] = analogRead(PIN_VOLTAGE);
        for (int c = 0; c < NUM_CHANNELS; c++)
            currentBuffer[c][i] = analogRead(currentPins[c]);

        while ((micros() - t) < SAMPLE_INTERVAL_US)
        {
            // Wait. NOTE: if this ever prints as consistently 0 wait time
            // (i.e. the 5 analogReads take longer than SAMPLE_INTERVAL_US),
            // the real sample rate is silently dropping below 4kHz -- worth
            // an occasional Serial check if you change SAMPLE_RATE upward.
        }
    }
}
