#include "modes/WeekdayMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

void WeekdayMode::begin() { pending = true; }

void WeekdayMode::handle()
{
    now = time(nullptr);
    if (localtime_r(&now, &local) && (weekday != local.tm_wday || pending))
    {
        pending = false;
        weekday = local.tm_wday;
        Display.fillFrame(false);
        KortlinjeFont font{};
        TextHandler(std::string(names[weekday]), font).draw();
    }
}
