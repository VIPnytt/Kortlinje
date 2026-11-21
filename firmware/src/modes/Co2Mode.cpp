#include "modes/Co2Mode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

void Co2Mode::begin() { pending = true; }

void Co2Mode::handle()
{
    if (pending)
    {
        pending = false;
        DisplayService::fillFrame(false);
        const KortlinjeFont font{};
        TextHandler(std::to_string(co2), font).draw();
        transmit();
    }
}

void Co2Mode::set(uint16_t _co2)
{
    if (_co2 != co2)
    {
        co2 = _co2;
        pending = true;
    }
}

void Co2Mode::transmit()
{
    JsonDocument doc{};
    doc["co2"].set(co2);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
