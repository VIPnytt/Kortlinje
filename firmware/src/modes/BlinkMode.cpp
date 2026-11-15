#include "modes/BlinkMode.h"

#include "services/DisplayService.h"

BlinkMode::~BlinkMode() { cancel_repeating_timer(&timer); }

void BlinkMode::begin() { add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 10U), &onTimer, this, &timer); }

/**
 * @brief Toggles the display state between all pixels lit and unlit on each timer tick.
 */
void BlinkMode::handle()
{
    if (tick)
    {
        tick = false;
        lit = !lit;
        DisplayService::fillFrame(lit);
    }
}

bool __time_critical_func(BlinkMode::onTimer)(repeating_timer *rt)
{
    auto *self{static_cast<BlinkMode *>(rt->user_data)};
    self->tick = true;
    return true;
}
