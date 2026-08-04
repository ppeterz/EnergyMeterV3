#include "Logger.h"
#include "Tariff.h"
#include <Arduino.h>

static const char* channelName(int c)
{
    if (c == 0) return "INPUT ";
    static char buf[8];
    snprintf(buf, sizeof(buf), "SOCK %d", c);
    return buf;
}

void Logger::print(const Measurement m[NUM_CHANNELS], Energy energy[NUM_CHANNELS])
{
    Serial.println("--------------------------------");
    Serial.printf("Voltage : %.2f V   |   Tariff Band %s @ N%.2f/kWh\n",
                  m[0].voltage, tariff.getBandName(), tariff.getRate());
    for (int c = 0; c < NUM_CHANNELS; c++)
    {
        Serial.printf("%s  I:%6.3fA  Real:%7.1fW  VA:%7.1fVA  PF:%.2f  E:%.4fkWh",
                      channelName(c), m[c].current, m[c].realPower,
                      m[c].apparentPower, m[c].powerFactor, energy[c].getkWh());
        if (c > 0)   // skip input channel for ₦ breakdown
            Serial.printf("  N%.2f", energy[c].getNaira());
        Serial.println();
    }
    float totalNaira = 0;
    for (int c = 1; c <= NUM_SOCKETS; c++) totalNaira += energy[c].getNaira();
    Serial.printf("TOTAL COST: N%.2f\n", totalNaira);
}

