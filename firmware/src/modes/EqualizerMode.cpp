#include "modes/EqualizerMode.h"

#include "services/DisplayService.h"

EqualizerMode::~EqualizerMode() { cancel_repeating_timer(&timer); }

void EqualizerMode::begin()
{
    for (uint8_t x{width}; x < GRID_COLUMNS; x += width + 1U)
    {
        for (uint8_t y{0U}; y < GRID_ROWS; ++y)
        {
            DisplayService::setPixel(x, y, false);
        }
    }
    add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 4U), &onTimer, this, &timer);
}

void EqualizerMode::handle()
{
    if (tick)
    {
        tick = false;
        uint8_t idx{0U};
        for (std::pair<uint8_t, uint8_t> &bar : bars)
        {
            if (bar.second == bar.first)
            {
                bar.second = static_cast<uint8_t>(random(GRID_ROWS));
            }
            else if (random(0b1U << 3U) == 0)
            {
                const uint8_t minX{static_cast<uint8_t>(idx * (width + 1U))};
                if (bar.first < bar.second)
                {
                    DisplayService::drawLineHorizontal(minX, width, bar.first, false);
                    ++bar.first;
                }
                else if (bar.first > bar.second)
                {
                    --bar.first;
                }
                DisplayService::drawRectangleSolid(minX, width, bar.first, GRID_ROWS - 1U, true);
            }
            ++idx;
        }
    }
}

bool __time_critical_func(EqualizerMode::onTimer)(repeating_timer *rt)
{
    static_cast<EqualizerMode *>(rt->user_data)->tick = true;
    return true;
}
