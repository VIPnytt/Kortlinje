#include "modes/DateMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

#include <string>

DateMode::~DateMode() { cancel_repeating_timer(&timer); }

void DateMode::begin() { add_repeating_timer_us(-static_cast<int64_t>(0b1U << 20U), &onTimer, this, &timer); }

void DateMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    now = time(nullptr);
    if (localtime_r(&now, &local) != nullptr && day != local.tm_mday)
    {
        day = local.tm_mday;
        month = local.tm_mon;
        DisplayService::fillFrame(false);
        const KortlinjeFont font{};
        TextHandler(months[static_cast<size_t>(month)], font).draw((Display::grid::columns / 2U) - 21U);
        const TextHandler mm2{TextHandler(std::to_string(day), font)};
        mm2.draw((Display::grid::columns / 2U) + 21U - mm2.getWidth());
    }
}

bool __time_critical_func(DateMode::onTimer)(repeating_timer *timer_data)
{
    static_cast<DateMode *>(timer_data->user_data)->tick = true;
    return true;
}
