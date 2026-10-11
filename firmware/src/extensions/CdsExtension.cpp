#include "extensions/CdsExtension.h"

#include "config/secrets.h"
#include "services/DisplayService.h"

void CdsExtension::begin()
{
    pinMode(PIN_CDS, PinMode::INPUT);
    attachInterrupt(PIN_CDS, &onChange, PinStatus::CHANGE);
}

void CdsExtension::handle()
{
    if (!changed && debounce == 0U)
    {
        return;
    }
    changed = false;
    switch (digitalRead(PIN_CDS))
    {
    case PinStatus::LOW:
        if (--debounce == INT8_MIN)
        {
            debounce = 0;
            state = false;
            Serial.println("CdsExtension::handle() - LOW");
            Display.setPower(false);
        }
        else if (debounce > 0)
        {
            debounce = 0;
        }
        break;
    case PinStatus::HIGH:
        if (++debounce == INT8_MAX)
        {
            debounce = 0;
            state = true;
            Serial.println("CdsExtension::handle() - HIGH");
            Display.setPower(true);
        }
        else if (debounce < 0)
        {
            debounce = 0;
        }
        break;
    }
}

void CdsExtension::onChange() { changed = true; }
