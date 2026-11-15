#include "modes/StarsMode.h"

#include "services/DisplayService.h"

StarsMode::~StarsMode() { cancel_repeating_timer(&timer); }

void StarsMode::begin()
{
    DisplayService::fillFrame(false);
    add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 6U), &onTimer, this, &timer);
}

/**
 * @brief Advances the star animation by toggling stars on and off with randomized delays.
 */
void StarsMode::handle()
{
    if (tick)
    {
        tick = false;
        for (Star &star : stars)
        {
            if (star.delay != 0U)
            {
                --star.delay;
            }
            else if (star.lit)
            {
                DisplayService::setPixel(star.idx, false);
                star.lit = false;
                star.delay = static_cast<uint8_t>(random(static_cast<long>(0b1U << 10U)));
            }
            else
            {
                do
                {
                    star.idx = static_cast<size_t>(random(static_cast<long>(GRID_COLUMNS * GRID_ROWS)));
                } while (DisplayService::getPixel(star.idx));
                DisplayService::setPixel(star.idx, true);
                star.lit = true;
                star.delay = static_cast<uint8_t>(random(static_cast<long>(0b1U << 12U)));
            }
        }
    }
}

bool __time_critical_func(StarsMode::onTimer)(repeating_timer *rt)
{
    static_cast<StarsMode *>(rt->user_data)->tick = true;
    return true;
}
