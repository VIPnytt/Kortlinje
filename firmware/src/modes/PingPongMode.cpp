#include "modes/PingPongMode.h"

#include "services/DeviceService.h"

#include <algorithm>
#include <cmath>

PingPongMode::~PingPongMode() { cancel_alarm(alarmId); }

void PingPongMode::begin()
{
    DisplayService::fillFrame(false);
    DisplayService::drawLineVertical(0U, paddleL - 1U, paddleL + 1U, true);
    DisplayService::drawLineVertical(Display::grid::maxX, paddleR - 1U, paddleR + 1U, true);
    DisplayService::setPixel(x, y, true);
    alarmId = add_alarm_in_us(interval, &onAlarm, this, true);
}

void PingPongMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if (xDec <= 1.0F && deg >= 90U && deg < 270U)
    {
        deg = (random(60L, 121L) + 270U) % 360U; // ±30°
    }
    else if (xDec >= static_cast<float>(Display::grid::maxX - 1U) && (deg < 90U || deg >= 270U))
    {
        deg = static_cast<uint16_t>(random(150L, 211L)); // ±30°
    }
    if (xDec <= .0F || xDec >= static_cast<float>(Display::grid::maxX))
    {
        deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
        xDec = std::clamp(xDec, .0F, static_cast<float>(Display::grid::maxX));
    }
    if (yDec <= .0F || yDec >= static_cast<float>(Display::grid::maxY))
    {
        deg = 360U - deg; // Invert Y
        yDec = std::clamp(yDec, .0F, static_cast<float>(Display::grid::maxY));
    }
    const float rad{static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)};
    const float dx{cosf(rad)};
    const float dy{-sinf(rad)};
    const float maxComponent{std::max(fabsf(dx), fabsf(dy))};
    interval = static_cast<int64_t>(static_cast<float>(0b1U << 15U) / maxComponent);
    DisplayService::setPixel(x, y, false);
    xDec += dx / maxComponent;
    yDec += dy / maxComponent;
    x = static_cast<uint8_t>(std::clamp<int>(lroundf(xDec), 0, static_cast<int>(Display::grid::maxX)));
    y = static_cast<uint8_t>(std::clamp<int>(lroundf(yDec), 0, static_cast<int>(Display::grid::maxY)));
    DisplayService::setPixel(x, y, true);
    const float lRad{atanf((xDec - 1.0F) / std::max(.1F, fabsf(static_cast<float>(paddleL) - yDec)))};
    const float rRad{atanf((static_cast<float>(Display::grid::maxX - 1U) - xDec) /
                           std::max(.1F, fabsf(static_cast<float>(paddleR) - yDec)))};
    if (yDec > paddleL + 1U && lRad < 1.3F && paddleL < Display::grid::maxY - 1U)
    {
        DisplayService::setPixel(0U, paddleL - 1U, false);
        ++paddleL;
        DisplayService::setPixel(0U, paddleL + 1U, true);
    }
    else if (yDec < paddleL - 1U && lRad < 1.3F && paddleL > 1U)
    {
        DisplayService::setPixel(0U, paddleL + 1U, false);
        --paddleL;
        DisplayService::setPixel(0U, paddleL - 1U, true);
    }
    if (yDec > paddleR + 1U && rRad < 1.3F && paddleR < Display::grid::maxY - 1U)
    {
        DisplayService::setPixel(Display::grid::maxX, paddleR - 1U, false);
        ++paddleR;
        DisplayService::setPixel(Display::grid::maxX, paddleR + 1U, true);
    }
    else if (yDec < paddleR - 1U && rRad < 1.3F && paddleR > 1U)
    {
        DisplayService::setPixel(Display::grid::maxX, paddleR + 1U, false);
        --paddleR;
        DisplayService::setPixel(Display::grid::maxX, paddleR - 1U, true);
    }
}

int64_t __time_critical_func(PingPongMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    PingPongMode *self{static_cast<PingPongMode *>(user_data)};
    self->tick = true;
    return self->interval;
}
