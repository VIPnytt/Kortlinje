#include "modes/RainMode.h"

#include "services/DisplayService.h"

#include <algorithm>

RainMode::~RainMode() { cancel_repeating_timer(&timer); }

void RainMode::begin() { add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 4U), &onTimer, this, &timer); }

/**
 * @brief Advances each raindrop at its own constant speed as its tick counter elapses.
 */
void RainMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    for (Drop &drop : drops)
    {
        if (++drop.ticks == drop.delay)
        {
            drop.ticks = 0U;
            if (drop.y >= GRID_ROWS + drop.length - 1U)
            {
                const uint8_t newX{static_cast<uint8_t>(random(static_cast<long>(GRID_COLUMNS)))};
                if (std::none_of(drops.begin(), drops.end(), [&](const Drop &_drop) { return _drop.x == newX; }))
                {
                    drop.x = newX;
                }
                drop.y = 0U;
                drop.length = static_cast<uint8_t>(random(2L, 5L));
                drop.delay = static_cast<uint8_t>(random(3L, 12L));
            }
            else
            {
                ++drop.y;
            }
            if (drop.y >= drop.length)
            {
                DisplayService::setPixel(drop.x, drop.y - drop.length, false);
            }
            if (drop.y < GRID_ROWS)
            {
                DisplayService::setPixel(drop.x, drop.y, true);
            }
        }
    }
}

bool __time_critical_func(RainMode::onTimer)(repeating_timer *rt)
{
    static_cast<RainMode *>(rt->user_data)->tick = true;
    return true;
}
