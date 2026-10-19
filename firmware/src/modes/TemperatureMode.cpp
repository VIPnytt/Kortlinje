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
        TextHandler(std::format("{:.1f}", temperature).append("°C"), font).draw();
    }
    else if (millis() - lastMillis > 0b1U << 14U)
    {
        lastMillis = millis();
        Extensions.Climate().sample();
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
