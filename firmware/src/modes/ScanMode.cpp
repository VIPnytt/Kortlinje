#include "modes/ScanMode.h"

#include "services/DisplayService.h"

ScanMode::~ScanMode() { cancel_repeating_timer(&timer); }

void ScanMode::begin() { add_repeating_timer_us(-static_cast<int64_t>(0b1U << 16U), &onTimer, this, &timer); }

void ScanMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if (x < Display::grid::columns)
    {
        DisplayService::fillColumn(x, true);
    }
    if (x >= 2U)
    {
        DisplayService::fillColumn(x - 2U, false);
    }
    ++x;
    if (x >= Display::grid::columns + 2U)
    {
        x = 0U;
    }
}

bool __time_critical_func(ScanMode::onTimer)(repeating_timer *rt)
{
    ScanMode *self{static_cast<ScanMode *>(rt->user_data)};
    self->tick = true;
    return true;
}
