#include "Relay.h"
#include "Config.h"

void Relay::begin(int p)
{
    pin = p;
    pinMode(pin, OUTPUT);
    off();
}

void Relay::on()
{
#if RELAY_ACTIVE_LOW
    digitalWrite(pin, LOW);
#else
    digitalWrite(pin, HIGH);
#endif
    state = true;
}

void Relay::off()
{
#if RELAY_ACTIVE_LOW
    digitalWrite(pin, HIGH);
#else
    digitalWrite(pin, LOW);
#endif
    state = false;
}

void Relay::toggle() { if (state) off(); else on(); }
bool Relay::isOn()   { return state; }
