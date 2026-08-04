#ifndef ENERGY_H
#define ENERGY_H

#include <Preferences.h>

class Energy
{
public:
    void begin(const char* storageKey);   // e.g. "eIn", "e1", "e2", "e3"

    void update(float realPower);

    float getWh();
    float getkWh();
    float getNaira();   // cost in ₦ using the active IBEDC tariff band

    void reset();
    void save();     // persist current value to flash -- call periodically, not every loop

private:
    Preferences prefs;
    const char* key;
    unsigned long lastUpdate;
    float energyWh;
};

#endif
