#include "extensions/PhotocellExtension.h"

#include "config/secrets.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <ArduinoJson.h>
#include <algorithm>
#include <hardware/gpio.h>
#include <hardware/timer.h>

void PhotocellExtension::begin()
{
    gpio_init(PIN_CDS);
    gpio_disable_pulls(PIN_CDS);
    gpio_set_input_enabled(PIN_CDS, true);
#ifdef RP2350_ERRATA_E9
    gpio_set_drive_strength(PIN_CDS, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PIN_CDS, false);
    gpio_set_dir(PIN_CDS, gpio_dir::GPIO_OUT);
    tight_loop_contents();
#endif // RP2350_ERRATA_E9
    gpio_set_dir(PIN_CDS, gpio_dir::GPIO_IN);
}

void PhotocellExtension::handle()
{
    if (pending)
    {
        pending = false;
        transmit();
    }
    else if (!active || millis() - lastMillis < 0b1UL << 6U)
    {
        return;
    }
    lastMillis = millis();
    const bool state{gpio_get(PIN_CDS)};
#ifdef RP2350_ERRATA_E9
    gpio_set_dir(PIN_CDS, gpio_dir::GPIO_OUT);
    tight_loop_contents();
    gpio_set_dir(PIN_CDS, gpio_dir::GPIO_IN);
#endif // RP2350_ERRATA_E9
    if (state && debounce != INT8_MAX)
    {
        ++debounce;
    }
    else if (!state && debounce != INT8_MIN)
    {
        --debounce;
    }
    else if (detected && debounce == INT8_MIN)
    {
        detected = false;
        pending = true;
        Display.setPower(false);
    }
    else if (!detected && debounce == INT8_MAX)
    {
        detected = true;
        pending = true;
        Display.setPower(true);
    }
}

bool PhotocellExtension::getActive() const { return active; }

bool PhotocellExtension::getDetected() const { return detected; }

void PhotocellExtension::setActive(bool _active)
{
    if (_active != active)
    {
        debounce = 0;
        active = _active;
        pending = true;
    }
}

void PhotocellExtension::transmit()
{
    JsonDocument doc{};
    doc["active"].set(active);
    doc["detected"].set(detected);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
