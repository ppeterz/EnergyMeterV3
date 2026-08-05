#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Preferences.h>
#include "Config.h"

class Calibration
{
public:
    void begin();

    float getVoltScale();
    float getCurrentScale(int channel = 0);

    void setVoltScale(float scale);
    void setCurrentScale(int channel, float scale);
    void setCurrentScaleAll(float scale);

private:
    Preferences prefs;
    float voltScale;
    float currentScale[NUM_CHANNELS];
};

extern Calibration calibration;   // single shared instance, used everywhere

#endif