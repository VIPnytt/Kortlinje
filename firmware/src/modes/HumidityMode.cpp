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
        Display.fillFrame(false);
        KortlinjeFont font{};
        TextHandler(std::to_string(humidity).append("%"), font).draw();
    }
}

void HumidityMode::set(uint8_t _humidity)
{
    if (_humidity != humidity)
    {
        humidity = _humidity;
        ConfigHandler::set(name, "humidity", humidity);
        pending = true;
    }
}
