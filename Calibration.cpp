#include "Calibration.h"
#include "Config.h"

Calibration calibration;

void Calibration::begin()
{
    prefs.begin("energy", false);

    voltScale    = prefs.getFloat("voltScale",    VOLT_SCALE);
    currentScale = prefs.getFloat("currentScale", CURRENT_SCALE);
}

float Calibration::getVoltScale()
{
    return voltScale;
}

float Calibration::getCurrentScale()
{
    return currentScale;
}

void Calibration::setVoltScale(float scale)
{
    voltScale = scale;
    prefs.putFloat("voltScale", scale);
}

void Calibration::setCurrentScale(float scale)
{
    currentScale = scale;
    prefs.putFloat("currentScale", scale);
}