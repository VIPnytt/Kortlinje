#include "modes/TemperatureMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

#include <format>

void TemperatureMode::begin() { pending = true; }

void TemperatureMode::handle()
{
    if (pending)
    {
        pending = false;
        Display.fillFrame(false);
        KortlinjeFont font{};
#if TEMPERATURE_FAHRENHEIT
        TextHandler(std::format("{:.0f}°F", temperature), font).draw();
#elif TEMPERATURE_KELVIN
        TextHandler(std::format("{:.0f}K", temperature), font).draw();
#else
        TextHandler(std::format("{:.1f}°C", temperature), font).draw();
#endif // TEMPERATURE_FAHRENHEIT
    }
    else if (millis() - lastMillis > 0b1U << 14U)
    {
        lastMillis = millis();
        Extensions.climate().sample();
    }
}

void TemperatureMode::set(float _temperature)
{
    lastMillis = millis();
    if (_temperature != temperature)
    {
        temperature = _temperature;
        pending = true;
    }
}
