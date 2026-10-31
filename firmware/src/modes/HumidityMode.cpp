#include "modes/HumidityMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

void HumidityMode::begin()
{
    ConfigHandler::get(name, "humidity", humidity);
    pending = true;
}

void HumidityMode::handle()
{
    if (pending)
    {
        pending = false;
        DisplayService::fillFrame(false);
        KortlinjeFont font{};
        TextHandler(std::format("{:d}%", humidity), font).draw();
        if (!saved)
        {
            saved = true;
            ConfigHandler::set(name, "humidity", humidity);
        }
        transmit();
    }
}

void HumidityMode::set(uint8_t _humidity)
{
    if (_humidity != humidity)
    {
        humidity = _humidity;
        saved = false;
        pending = true;
    }
}

void HumidityMode::transmit()
{
    JsonDocument doc{};
    doc["humidity"].set(humidity);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
