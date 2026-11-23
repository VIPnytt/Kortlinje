#include "modes/WeekdayMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

#include <string>

WeekdayMode::~WeekdayMode() { cancel_repeating_timer(&timer); }

void WeekdayMode::begin() { add_repeating_timer_us(-static_cast<int64_t>(0b1U << 20U), &onTimer, this, &timer); }

void WeekdayMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    now = time(nullptr);
    if (localtime_r(&now, &local) != nullptr && weekday != local.tm_wday)
    {
        weekday = local.tm_wday;
        DisplayService::fillFrame(false);
        const KortlinjeFont font{};
        TextHandler(weekdays[static_cast<size_t>(weekday)], font).draw();
    }
}

bool __time_critical_func(WeekdayMode::onTimer)(repeating_timer *timer_data)
{
    static_cast<WeekdayMode *>(timer_data->user_data)->tick = true;
    return true;
}
