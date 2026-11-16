#include "modes/BlindsMode.h"

BlindsMode::~BlindsMode() { cancel_repeating_timer(&timer); }

void BlindsMode::begin() { add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 8U), &onTimer, this, &timer); }

/**
 * @brief Advances the blinds animation after the update interval elapses.
 */
void BlindsMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if (modulo <= 1U)
    {
        DisplayService::fillFrame(true);
        direction = true;
        ++modulo;
    }
    else
    {
        DisplayService::fillFrame(false);
        for (uint8_t x{static_cast<uint8_t>((modulo + (Display::grid::columns % modulo) - 1U) / 2U)};
             x < Display::grid::columns;
             x += modulo)
        {
            DisplayService::fillColumn(x, true);
        }
        if (modulo >= moduloMax)
        {
            direction = false;
        }
        modulo = direction ? static_cast<uint8_t>(Display::grid::columns / (Display::grid::columns / (modulo + 1U)))
                           : static_cast<uint8_t>(Display::grid::columns / ((Display::grid::columns / modulo) + 1U));
    }
}

bool __time_critical_func(BlindsMode::onTimer)(repeating_timer *rt)
{
    static_cast<BlindsMode *>(rt->user_data)->tick = true;
    return true;
}
