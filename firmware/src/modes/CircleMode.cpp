#include "modes/CircleMode.h"

#include "services/DisplayService.h"

CircleMode::~CircleMode() { cancel_repeating_timer(&timer); }

void CircleMode::begin() { add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 6U), &onTimer, this, &timer); }

/**
 * @brief Advances the expanding concentric-circle animation on each timer tick.
 */
void CircleMode::handle()
{
    if (tick)
    {
        tick = false;
        bool _lit{lit};
        for (uint8_t _radius{radius}; _radius <= maxRadius; _radius += 3U)
        {
            DisplayService::drawEllipseOutline(x, y, static_cast<float>(_radius), _lit);
            _lit = !_lit;
        }
        ++radius;
        if (radius > 3U)
        {
            radius = 1U;
            lit = !lit;
        }
    }
}

bool __time_critical_func(CircleMode::onTimer)(repeating_timer *rt)
{
    static_cast<CircleMode *>(rt->user_data)->tick = true;
    return true;
}
