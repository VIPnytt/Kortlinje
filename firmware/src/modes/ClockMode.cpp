#include "modes/ClockMode.h"

#include "constants/Kortlinje.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/BitmapHandler.h"
#include "services/DisplayService.h"

void ClockMode::begin() { pending = true; }

void ClockMode::handle()
{
    now = time(nullptr);
    if (localtime_r(&now, &local) != nullptr && (minute != local.tm_min || hour != local.tm_hour || pending))
    {
        pending = false;
        hour = local.tm_hour;
        minute = local.tm_min;
        DisplayService::fillFrame(false);
        const int _hour{Kortlinje::time::format == 12U ? (local.tm_hour + 11) % 12 + 1 : local.tm_hour};
        if constexpr (Kortlinje::time::format == 12U)
        {
            DisplayService::setPixel(local.tm_hour < 12 ? DisplayService::Symbol::AM : DisplayService::Symbol::PM,
                                     true);
            if (_hour >= 10)
            {
                BitmapHandler(KortlinjeFont::digitZero_digitNine[1U])
                    .draw((Display::grid::columns / 2U) - 20U, (Display::grid::rows / 2U) - 4U);
            }
        }
        else
        {
            BitmapHandler(KortlinjeFont::digitZero_digitNine[_hour / 10])
                .draw(_hour / 10 == 1 ? (Display::grid::columns / 2U) - 20U : (Display::grid::columns / 2U) - 21U,
                      (Display::grid::rows / 2U) - 4U);
        }
        BitmapHandler(KortlinjeFont::digitZero_digitNine[_hour % 10])
            .draw((Display::grid::columns / 2U) - 12U, (Display::grid::rows / 2U) - 4U);
        BitmapHandler(KortlinjeFont::colon).draw((Display::grid::columns / 2U) - 1U, (Display::grid::rows / 2U) - 3U);
        BitmapHandler(KortlinjeFont::digitZero_digitNine[minute / 10])
            .draw(minute / 10 == 1 ? (Display::grid::columns / 2U) + 6U : (Display::grid::columns / 2U) + 5U,
                  (Display::grid::rows / 2U) - 4U);
        BitmapHandler(KortlinjeFont::digitZero_digitNine[minute % 10])
            .draw((Display::grid::columns / 2U) + 14U, (Display::grid::rows / 2U) - 4U);
    }
}
