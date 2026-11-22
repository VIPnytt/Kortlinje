#include "modes/PingPongMode.h"

#include "services/DisplayService.h"

#include <algorithm>
#include <cmath>
#include <numbers>

PingPongMode::~PingPongMode() { cancel_alarm(alarmId); }

void PingPongMode::begin()
{
    DisplayService::fillFrame(false);
    DisplayService::drawLineVertical(0U, paddleL - 1U, paddleL + 1U, true);
    DisplayService::drawLineVertical(Display::grid::maxX, paddleR - 1U, paddleR + 1U, true);
    DisplayService::setPixel(x, y, true);
    alarmId = add_alarm_in_us(static_cast<uint64_t>(interval), &onAlarm, this, true);
}

void PingPongMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if ((y == 0U && stepY < .0F) || (y >= Display::grid::maxY && stepY > .0F))
    {
        stepY = -stepY;
    }
    if ((x <= 1U && stepX < 0) || (x >= Display::grid::maxX - 1U && stepX > 0))
    {
        stepX = -stepX;
        stepY = (static_cast<float>(random(101L)) * 1e-2F) -
                (static_cast<float>(y) * (1.0F / static_cast<float>(Display::grid::maxY)));
        interval = static_cast<int64_t>(static_cast<float>(0b1U << 15U) * sqrtf(1.0F + (stepY * stepY)));
    }
    DisplayService::setPixel(x, y, false);
    x = static_cast<uint8_t>(std::clamp(static_cast<int>(x) + stepX, 1, static_cast<int>(Display::grid::maxX - 1U)));
    yDec = std::clamp(yDec + stepY, .0F, static_cast<float>(Display::grid::maxY));
    y = static_cast<uint8_t>(std::lroundf(yDec));
    DisplayService::setPixel(x, y, true);
    const float distXL{static_cast<float>(x - 1U)};
    const float distYL{fabsf(static_cast<float>(paddleL) - yDec)};
    if (stepX < 0 && distXL < distYL * std::numbers::pi_v<float>)
    {
        if (paddleL > 1U && yDec < static_cast<float>(paddleL - 1U))
        {
            DisplayService::setPixel(0U, paddleL + 1U, false);
            --paddleL;
            DisplayService::setPixel(0U, paddleL - 1U, true);
        }
        else if (paddleL < Display::grid::maxY - 1U && yDec > static_cast<float>(paddleL + 1U))
        {
            DisplayService::setPixel(0U, paddleL - 1U, false);
            ++paddleL;
            DisplayService::setPixel(0U, paddleL + 1U, true);
        }
    }
    const float distXR{static_cast<float>((Display::grid::maxX - 1U) - x)};
    const float distYR{fabsf(static_cast<float>(paddleR) - yDec)};
    if (stepX > 0 && distXR < distYR * std::numbers::pi_v<float>)
    {
        if (paddleR > 1U && yDec < static_cast<float>(paddleR - 1U))
        {
            DisplayService::setPixel(Display::grid::maxX, paddleR + 1U, false);
            --paddleR;
            DisplayService::setPixel(Display::grid::maxX, paddleR - 1U, true);
        }
        else if (paddleR < Display::grid::maxY - 1U && yDec > static_cast<float>(paddleR + 1U))
        {
            DisplayService::setPixel(Display::grid::maxX, paddleR - 1U, false);
            ++paddleR;
            DisplayService::setPixel(Display::grid::maxX, paddleR + 1U, true);
        }
    }
}

int64_t __time_critical_func(PingPongMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    PingPongMode *self{static_cast<PingPongMode *>(user_data)};
    self->tick = true;
    return self->interval;
}
