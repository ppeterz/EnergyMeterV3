#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Preferences.h>

class Calibration
{
public:
    void begin();

    float getVoltScale();
    float getCurrentScale();

    void setVoltScale(float scale);
    void setCurrentScale(float scale);

private:
    Preferences prefs;
    float voltScale;
    float currentScale;
};

extern Calibration calibration;   // single shared instance, used everywhere

#endif