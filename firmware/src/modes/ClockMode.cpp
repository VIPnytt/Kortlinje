#include "modes/ClockMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

void ClockMode::begin() { pending = true; }

void ClockMode::handle()
{
    now = time(nullptr);
    if (localtime_r(&now, &local) && (minute != local.tm_min || hour != local.tm_hour || pending))
    {
        pending = false;
        hour = local.tm_hour;
        minute = local.tm_min;
        Display.fillFrame(false);
        KortlinjeFont font{};
        TextHandler(std::to_string(hour / 10)
                        .append(std::to_string(hour % 10))
                        .append(":")
                        .append(std::to_string(minute / 10))
                        .append(std::to_string(minute % 10)),
                    font)
            .draw();
    }
}
