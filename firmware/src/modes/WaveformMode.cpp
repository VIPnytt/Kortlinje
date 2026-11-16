#include "modes/WaveformMode.h"

#include <algorithm>

WaveformMode::~WaveformMode() { cancel_repeating_timer(&timer); }

void WaveformMode::begin()
{
    wavePhase = static_cast<float>(random(360L)) * static_cast<float>(DEG_TO_RAD);
    ampPhase = static_cast<float>(random(360L)) * static_cast<float>(DEG_TO_RAD);
    freqPhase = static_cast<float>(random(360L)) * static_cast<float>(DEG_TO_RAD);
    for (uint8_t x{0U}; x < Display::grid::columns; ++x)
    {
        renderColumn(x);
    }
    add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 6U), &onTimer, this, &timer);
}

/**
 * @brief Shifts existing columns left and appends a new wave sample on the right edge.
 */
void WaveformMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    for (uint8_t x{0U}; x < Display::grid::maxX; ++x)
    {
        for (uint8_t y{0U}; y < Display::grid::rows; ++y)
        {
            DisplayService::setPixel(x, y, DisplayService::getPixel(x + 1U, y));
        }
    }
    DisplayService::fillColumn(Display::grid::maxX, false);
    renderColumn(Display::grid::maxX);
}

bool __time_critical_func(WaveformMode::onTimer)(repeating_timer *rt)
{
    static_cast<WaveformMode *>(rt->user_data)->tick = true;
    return true;
}

void WaveformMode::renderColumn(uint8_t x)
{
    const float amp{baseAmp + (ampDrift * sinf(ampPhase))};
    const float freq{baseFreq + (freqDrift * sinf(freqPhase))};
    const float yFloat{centerY + (amp * sinf(wavePhase))};
    const uint8_t y{static_cast<uint8_t>(std::clamp<int>(lroundf(yFloat), 0, static_cast<int>(Display::grid::maxY)))};
    DisplayService::setPixel(x, y, true);
    wavePhase = wrapPhase(wavePhase + freq);
    ampPhase = wrapPhase(ampPhase + .053F);
    freqPhase = wrapPhase(freqPhase + .037F);
}

float WaveformMode::wrapPhase(float phase) { return phase >= twoPi ? phase - twoPi : phase; }
