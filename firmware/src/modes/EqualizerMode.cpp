#include "modes/EqualizerMode.h"

#include "services/DisplayService.h"

EqualizerMode::~EqualizerMode() { cancel_repeating_timer(&timer); }

void EqualizerMode::begin()
{
    uint8_t x{width};
    for (; x < Display::grid::columns - width; x += width + 1U)
    {
        DisplayService::fillColumn(x, false);
    }
    DisplayService::fillColumns(x, Display::grid::columns - x, false);
    bars.fill({Display::grid::maxY, Display::grid::maxY});
    add_repeating_timer_us(-static_cast<int64_t>(0b1U << 14U), &onTimer, this, &timer);
}

void EqualizerMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    uint8_t idx{0U};
    for (std::pair<uint8_t, uint8_t> &bar : bars)
    {
        if (bar.second == bar.first)
        {
            bar.second = static_cast<uint8_t>(random(static_cast<long>(Display::grid::rows)));
        }
        else if (random(static_cast<long>(0b1U << 3U)) == 0L)
        {
            const uint8_t minX{static_cast<uint8_t>(idx * (width + 1U))};
            if (bar.first < bar.second)
            {
                DisplayService::drawLineHorizontal(minX, width, bar.first, false);
                ++bar.first;
            }
            else if (bar.first > bar.second)
            {
                DisplayService::drawLineHorizontal(minX, width, bar.first, true);
                --bar.first;
            }
        }
        ++idx;
    }
}

bool __time_critical_func(EqualizerMode::onTimer)(repeating_timer *rt)
{
    static_cast<EqualizerMode *>(rt->user_data)->tick = true;
    return true;
}
