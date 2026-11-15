#include "modes/LinesMode.h"

#include "config/IkeaKortlinje.h"
#include "services/DisplayService.h"

LinesMode::~LinesMode() { cancel_repeating_timer(&timer); }

void LinesMode::begin() { add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 8U), &onTimer, this, &timer); }

/**
 * @brief Advances the line animation when its update interval has elapsed.
 */
void LinesMode::handle()
{
    if (tick)
    {
        tick = false;
        bool lit{x < 2U};
        for (uint8_t col{static_cast<uint8_t>(x & 0b1U)}; col < GRID_COLUMNS; col += 2U)
        {
            DisplayService::fillColumn(col, lit);
            lit = !lit;
        }
        x = (x + 1U) & 0b11U;
    }
}

bool __time_critical_func(LinesMode::onTimer)(repeating_timer *rt)
{
    static_cast<LinesMode *>(rt->user_data)->tick = true;
    return true;
}
