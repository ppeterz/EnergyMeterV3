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

private:
    // If the screen stays blank, 0x27 is the first thing to check -- run an
    // I2C scanner sketch once; 0x27 and 0x3F are the two common addresses.
    LiquidCrystal_I2C lcd{0x27, 16, 2};
};

#endif
