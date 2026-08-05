#include "Calibration.h"
#include "Config.h"

Calibration calibration;

void Calibration::begin()
{
    prefs.begin("energy", false);

    voltScale = prefs.getFloat("voltScale", VOLT_SCALE);

    char key[16];
    for (int c = 0; c < NUM_CHANNELS; c++)
    {
        snprintf(key, sizeof(key), "cScale%d", c);
        currentScale[c] = prefs.getFloat(key, CURRENT_SCALE);
    }
}

float Calibration::getVoltScale()
{
    return voltScale;
}

float Calibration::getCurrentScale(int channel)
{
    if (channel < 0 || channel >= NUM_CHANNELS) return currentScale[0];
    return currentScale[channel];
}

void Calibration::setVoltScale(float scale)
{
    voltScale = scale;
    prefs.putFloat("voltScale", scale);
}

void Calibration::setCurrentScale(int channel, float scale)
{
    if (channel < 0 || channel >= NUM_CHANNELS) return;
    currentScale[channel] = scale;
    char key[16];
    snprintf(key, sizeof(key), "cScale%d", channel);
    prefs.putFloat(key, scale);
}

void Calibration::setCurrentScaleAll(float scale)
{
    for (int c = 0; c < NUM_CHANNELS; c++)
    {
        setCurrentScale(c, scale);
    }
}