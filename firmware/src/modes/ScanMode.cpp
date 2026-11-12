#include "modes/ScanMode.h"

#include "services/DisplayService.h"
#include "services/ExtensionsService.h"

ScanMode::~ScanMode() { stopTimer(); }

void ScanMode::begin() { startTimer(); }

void ScanMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if (x < GRID_COLUMNS)
    {
        DisplayService::fillColumn(x, true);
    }
    if (x >= 2U)
    {
        DisplayService::fillColumn(x - 2U, false);
    }
    ++x;
    if (x >= GRID_COLUMNS + 2U)
    {
        x = 0U;
    }
}

void ScanMode::end() { stopTimer(); }

bool __time_critical_func(ScanMode::onTimer)(repeating_timer *rt)
{
    ScanMode *self{static_cast<ScanMode *>(rt->user_data)};
    self->tick = true;
    return true;
}

void ScanMode::startTimer()
{
    if (!timerRunning)
    {
        timerRunning = add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 6U), &onTimer, this, &timer);
    }
}

void ScanMode::stopTimer()
{
    if (timerRunning)
    {
        cancel_repeating_timer(&timer);
        timerRunning = false;
        tick = false;
    }
}
