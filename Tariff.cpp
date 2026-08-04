#include "Tariff.h"
#include <Arduino.h>

// Define the static constexpr arrays
constexpr float Tariff::rates[];
constexpr const char* Tariff::names[];

// Global instance
Tariff tariff;

void Tariff::begin()
{
    prefs.begin("tariff", false);
    uint8_t saved = prefs.getUChar("band", BAND_A);
    if (saved >= NUM_BANDS) saved = BAND_A;
    currentBand = static_cast<TariffBand>(saved);
}

void Tariff::setBand(TariffBand band)
{
    if (band >= NUM_BANDS) return;
    currentBand = band;
    prefs.putUChar("band", currentBand);
}

TariffBand Tariff::getBand()       const { return currentBand; }
const char* Tariff::getBandName()  const { return names[currentBand]; }
float Tariff::getRate()            const { return rates[currentBand]; }

float Tariff::kWhToNaira(float kWh) const
{
    return kWh * rates[currentBand];
}
