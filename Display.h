#ifndef DISPLAY_H
#define DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include "Measurement.h"
#include "Energy.h"
#include "Config.h"

class Display
{
public:
    void begin();
    void show(int screen, const Measurement m[NUM_CHANNELS], Energy energy[NUM_CHANNELS]);
    void showMessage(const char* line1, const char* line2 = nullptr);
    void resetActivityTimer();
    void checkPowerSave(unsigned long timeoutMs = 60000);

private:
    LiquidCrystal_I2C lcd{0x27, 16, 2};
    unsigned long lastActivityMs = 0;
    bool backlightState = true;
};

#endif
