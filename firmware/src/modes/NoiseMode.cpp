#include "modes/NoiseMode.h"

NoiseMode::~NoiseMode() { cancel_repeating_timer(&timer); }

void NoiseMode::begin()
{
    it = dots.begin();
    add_repeating_timer_ms(-5, &onTimer, this, &timer);
}

/**
 * @brief Randomly relocates a subset of noise dots on each timer tick.
 */
void NoiseMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    DisplayService::setPixel(*it, false);
    *it = static_cast<size_t>(random(static_cast<long>(Display::grid::pixels)));
    DisplayService::setPixel(*it, true);
    if (++it == dots.end())
    {
        it = dots.begin();
    }
}

bool __time_critical_func(NoiseMode::onTimer)(repeating_timer *rt)
{
    static_cast<NoiseMode *>(rt->user_data)->tick = true;
    return true;
}
