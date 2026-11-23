#include "modes/ClockMode.h"

#include "constants/Kortlinje.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/BitmapHandler.h"
#include "services/DisplayService.h"

ClockMode::~ClockMode() { cancel_repeating_timer(&timer); }

void ClockMode::begin() { add_repeating_timer_us(-static_cast<int64_t>(0b1U << 20U), &onTimer, this, &timer); }

void ClockMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    now = time(nullptr);
    if (localtime_r(&now, &local) != nullptr && minute != local.tm_min)
    {
        hour = local.tm_hour;
        minute = local.tm_min;
        DisplayService::fillFrame(false);
        if constexpr (Kortlinje::time::format == 12U)
        {
            DisplayService::setPixel(local.tm_hour < 12 ? DisplayService::Symbol::AM : DisplayService::Symbol::PM,
                                     true);
            const int _hour{((local.tm_hour + 11) % 12) + 1};
            if (_hour >= 10)
            {
                BitmapHandler(KortlinjeFont::digitZero_digitNine[1U]).draw((Display::grid::columns / 2U) - 20U);
            }
            BitmapHandler(KortlinjeFont::digitZero_digitNine[_hour % 10]).draw((Display::grid::columns / 2U) - 12U);
        }
        else
        {
            BitmapHandler(KortlinjeFont::digitZero_digitNine[local.tm_hour / 10])
                .draw((Display::grid::columns / 2U) - (local.tm_hour / 10 == 1 ? 20U : 21U));
            BitmapHandler(KortlinjeFont::digitZero_digitNine[local.tm_hour % 10])
                .draw((Display::grid::columns / 2U) - 12U);
        }
        BitmapHandler(KortlinjeFont::colon).draw((Display::grid::columns / 2U) - 1U);
        BitmapHandler(KortlinjeFont::digitZero_digitNine[minute / 10])
            .draw((Display::grid::columns / 2U) + (minute / 10 == 1 ? 6U : 5U));
        BitmapHandler(KortlinjeFont::digitZero_digitNine[minute % 10]).draw((Display::grid::columns / 2U) + 14U);
    }
}

bool __time_critical_func(ClockMode::onTimer)(repeating_timer *timer_data)
{
    static_cast<ClockMode *>(timer_data->user_data)->tick = true;
    return true;
}
