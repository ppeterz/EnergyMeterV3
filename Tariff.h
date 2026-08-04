#ifndef TARIFF_H
#define TARIFF_H

#include <Preferences.h>

// IBEDC Service-Based Tariff bands (prices include 7.5% VAT)
enum TariffBand : uint8_t
{
    BAND_A = 0,   // 20+ hrs/day  -- cost-reflective
    BAND_B = 1,   // 16+ hrs/day
    BAND_C = 2,   // 12+ hrs/day
    BAND_D = 3,   //  8+ hrs/day
    BAND_E = 4,   //  4+ hrs/day
    NUM_BANDS = 5
};

class Tariff
{
public:
    void begin();                       // load saved band from flash (defaults to BAND_A)

    void setBand(TariffBand band);      // change & persist
    TariffBand getBand() const;
    const char* getBandName() const;    // "A", "B", etc.
    float getRate() const;              // ₦ per kWh (inc. VAT)

    float kWhToNaira(float kWh) const;  // convenience

private:
    Preferences prefs;
    TariffBand currentBand = BAND_A;

    // Rates in ₦/kWh including the 7.5% VAT
    static constexpr float rates[NUM_BANDS] = {
        225.21f,   // Band A
         67.73f,   // Band B
         53.75f,   // Band C
         46.23f,   // Band D
         43.00f    // Band E
    };

    static constexpr const char* names[NUM_BANDS] = {
        "A", "B", "C", "D", "E"
    };
};

extern Tariff tariff;   // single global instance (defined in Tariff.cpp)

#endif
