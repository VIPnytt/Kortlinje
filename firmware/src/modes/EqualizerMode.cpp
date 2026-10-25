#include "modes/EqualizerMode.h"

#include "services/DisplayService.h"

EqualizerMode::~EqualizerMode() { stopTimer(); }

void EqualizerMode::begin()
{
    for (uint8_t x{width}; x < GRID_COLUMNS; x += width + 1U)
    {
        for (uint8_t y{0U}; y < GRID_ROWS; ++y)
        {
            Display.setPixel(x, y, false);
        }
    }
    startTimer();
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
                    Display.drawLineHorizontal(minX, width, bar.first, false);
                    ++bar.first;
                }
                else if (bar.first > bar.second)
                {
                    --bar.first;
                }
                Display.drawRectangleSolid(minX, width, bar.first, GRID_ROWS - 1U, true);
            }
            ++idx;
        }
    }
}

void EqualizerMode::end() { stopTimer(); }

bool __time_critical_func(EqualizerMode::onTimer)(repeating_timer *rt)
{
    tick = true;
    return true;
}

void EqualizerMode::startTimer()
{
    if (!timerRunning)
    {
        timerRunning = add_repeating_timer_ms(-(0b1L << 4U), &onTimer, nullptr, &timer);
    }
}

void EqualizerMode::stopTimer()
{
    if (timerRunning)
    {
        cancel_repeating_timer(&timer);
        timerRunning = false;
        tick = false;
    }
}
