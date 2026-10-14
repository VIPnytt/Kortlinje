#include "modes/DateMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

void DateMode::begin() { pending = true; }

void DateMode::handle()
{
    now = time(nullptr);
    if (localtime_r(&now, &local) && (day != local.tm_mday || month != local.tm_mon || pending))
    {
        pending = false;
        day = local.tm_mday;
        month = local.tm_mon;
        Display.fillFrame(false);
        KortlinjeFont font{};
        TextHandler(std::string(names[month]).append(" ").append(std::to_string(day)), font).draw();
    }
}
