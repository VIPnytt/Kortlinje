#include "modes/ClockMode.h"

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
        Display.fillFrame(false);
#if CLOCK_12H
        const int hour{(local.tm_hour + 11) % 12 + 1};
        Display.setPixel(local.tm_hour < 12 ? DisplayService::Symbol::AM : DisplayService::Symbol::PM, true);
        if (hour >= 10)
        {
            BitmapHandler(KortlinjeFont::digitZero_digitNine[1U])
                .draw((GRID_COLUMNS / 2U) - 20U, (GRID_ROWS / 2U) - 4U);
        }
#else
        BitmapHandler(KortlinjeFont::digitZero_digitNine[hour / 10])
            .draw(hour / 10 == 1 ? (GRID_COLUMNS / 2U) - 20U : (GRID_COLUMNS / 2U) - 21U, (GRID_ROWS / 2U) - 4U);
#endif // CLOCK_12H
        BitmapHandler(KortlinjeFont::digitZero_digitNine[hour % 10])
            .draw((GRID_COLUMNS / 2U) - 12U, (GRID_ROWS / 2U) - 4U);
        BitmapHandler(KortlinjeFont::colon).draw((GRID_COLUMNS / 2U) - 1U, (GRID_ROWS / 2U) - 3U);
        BitmapHandler(KortlinjeFont::digitZero_digitNine[minute / 10])
            .draw(minute / 10 == 1 ? (GRID_COLUMNS / 2U) + 6U : (GRID_COLUMNS / 2U) + 5U, (GRID_ROWS / 2U) - 4U);
        BitmapHandler(KortlinjeFont::digitZero_digitNine[minute % 10])
            .draw((GRID_COLUMNS / 2U) + 14U, (GRID_ROWS / 2U) - 4U);
    }
}
