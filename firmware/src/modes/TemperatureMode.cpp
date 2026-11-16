#include "modes/TemperatureMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

#include <format>

void TemperatureMode::begin()
{
    ConfigHandler::get(name, "temperature", temperature);
    pending = true;
}

void TemperatureMode::handle()
{
    if (pending)
    {
        pending = false;
        DisplayService::fillFrame(false);
        const KortlinjeFont font{};
        if constexpr (Kortlinje::device::temperature_unit == "°C")
        {
            TextHandler(std::format("{:.1f}°C", temperature), font).draw();
        }
        else if constexpr (Kortlinje::device::temperature_unit == "°F")
        {
            TextHandler(std::format("{:.0f}°F", temperature), font).draw();
        }
        else
        {
            TextHandler(std::format("{:.0f}K", temperature), font).draw();
        }
        if (!saved)
        {
            saved = true;
            ConfigHandler::set(name, "temperature", temperature);
        }
        transmit();
    }
}

void TemperatureMode::set(float _temperature)
{
    if (_temperature != temperature)
    {
        temperature = _temperature;
        saved = false;
        pending = true;
    }
}

void TemperatureMode::transmit()
{
    JsonDocument doc{};
    doc["temperature"].set(temperature);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
