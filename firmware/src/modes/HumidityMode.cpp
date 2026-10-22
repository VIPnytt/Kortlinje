#include "modes/HumidityMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

void HumidityMode::begin() { pending = true; }

void HumidityMode::handle()
{
    if (pending)
    {
        pending = false;
        Display.fillFrame(false);
        KortlinjeFont font{};
        TextHandler(std::to_string(humidity).append("%"), font).draw();
    }
    else if (millis() - lastMillis > 0b1U << 14U)
    {
        lastMillis = millis();
        Extensions.climate().sample();
    }
}

void HumidityMode::set(uint8_t _humidity)
{
    lastMillis = millis();
    if (_humidity != humidity)
    {
        humidity = _humidity;
        pending = true;
    }
}
