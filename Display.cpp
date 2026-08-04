#include "Display.h"
#include "Tariff.h"
#include <Wire.h>

void Display::begin()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Energy Monitor");
    lcd.setCursor(0, 1);
    lcd.print("Starting...");
}

// screen 0 = input (total), screen 1..NUM_SOCKETS = that socket
void Display::show(int screen, const Measurement m[NUM_CHANNELS], Energy energy[NUM_CHANNELS])
{
    lcd.clear();
    if (screen == 0)
    {
        // Line 1: voltage + tariff band
        lcd.setCursor(0, 0);
        lcd.printf("V:%3.0fV Band %s", m[0].voltage, tariff.getBandName());
        // Line 2: total ₦ cost across all sockets
        float totalNaira = 0;
        for (int c = 1; c <= NUM_SOCKETS; c++) totalNaira += energy[c].getNaira();
        lcd.setCursor(0, 1);
        if (totalNaira < 1000.0f)
            lcd.printf("Total: N%.2f", totalNaira);
        else
            lcd.printf("Total: N%.1f", totalNaira);
    }
    else
    {
        int c = screen;
        // Line 1: socket label + current
        lcd.setCursor(0, 0);
        lcd.printf("S%d I:%4.2fA", c, m[c].current);
        // Line 2: power + ₦ cost for this socket
        lcd.setCursor(0, 1);
        float naira = energy[c].getNaira();
        if (naira < 100.0f)
            lcd.printf("P:%4.0fW N%.2f", m[c].realPower, naira);
        else
            lcd.printf("P:%4.0fW N%.1f", m[c].realPower, naira);
    }
}

