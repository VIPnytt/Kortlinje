#include "modes/GlitterMode.h"

#include "services/DisplayService.h"

GlitterMode::~GlitterMode() { cancel_repeating_timer(&timer); }

void GlitterMode::begin() { add_repeating_timer_us(-static_cast<int64_t>(0b1U << 12U), &onTimer, this, &timer); }

void GlitterMode::handle()
{
    if (tick)
    {
        tick = false;
        const size_t idx{static_cast<size_t>(random(static_cast<long>(Display::grid::pixels)))};
        DisplayService::setPixel(idx, !DisplayService::getPixel(idx));
    }
}

bool __time_critical_func(GlitterMode::onTimer)(repeating_timer *rt)
{
    static_cast<GlitterMode *>(rt->user_data)->tick = true;
    return true;
}
