#include "extensions/PhotocellExtension.h"

#include "config/secrets.h"
#include "modes/TickerMode.h"
#include "services/DisplayService.h"

void PhotocellExtension::begin() { pinMode(PIN_CDS, PinMode::INPUT); }

void PhotocellExtension::handle()
{
    if (active && millis() - lastMillis > 0b1U << 13U)
    {
        process();
        lastMillis = millis();
    }
}

void PhotocellExtension::process()
{
    if (detected && digitalRead(PIN_CDS) == PinStatus::LOW)
    {
        detected = false;
        Display.setPower(false);
    }
    else if (!detected && digitalRead(PIN_CDS) == PinStatus::HIGH)
    {
        detected = true;
        Display.setPower(true);
    }
}

bool PhotocellExtension::getActive() const { return active; }

void PhotocellExtension::setActive(bool _active) { active = _active; }
