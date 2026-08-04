#include <Arduino.h>
#include "Energy.h"
#include "Tariff.h"

void Energy::begin(const char* storageKey)
{
    key = storageKey;
    prefs.begin("energy", false);
    energyWh = prefs.getFloat(key, 0.0f);   // survives reboot/power loss
    lastUpdate = millis();
}

void Energy::update(float realPower)
{
    unsigned long now = millis();
    float dtHours = (now - lastUpdate) / 3600000.0f;
    energyWh += realPower * dtHours;
    lastUpdate = now;
}

float Energy::getWh()  { return energyWh; }
float Energy::getkWh() { return energyWh / 1000.0f; }

float Energy::getNaira()
{
    return tariff.kWhToNaira(getkWh());
}

void Energy::reset()
{
    energyWh = 0;
    prefs.putFloat(key, energyWh);
}

void Energy::save()
{
    prefs.putFloat(key, energyWh);
}
