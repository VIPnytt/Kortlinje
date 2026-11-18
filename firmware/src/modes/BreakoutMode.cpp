#include "modes/BreakoutMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"

#include <algorithm>
#include <cmath>

BreakoutMode::~BreakoutMode() { cancel_alarm(alarmId); }

void BreakoutMode::begin()
{
    DisplayService::fillFrame(true);
    DisplayService::fillColumns(Display::grid::columns - 3U, 3U, false);
    DisplayService::drawLineVertical(Display::grid::maxX, paddleY - 1U, paddleY + 1U, true);
    DisplayService::setPixel(x, y, true);
    alarmId = add_alarm_in_us(static_cast<uint64_t>(interval), &onAlarm, this, true);
}

void BreakoutMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if (x == Display::grid::maxX - 1U)
    {
        now = time(nullptr);
        if (localtime_r(&now, &local) != nullptr && (minute != local.tm_min || hour != local.tm_hour))
        {
            hour = local.tm_hour;
            minute = local.tm_min;
            const int _hour{Kortlinje::time::format == 12U ? (local.tm_hour + 11) % 12 + 1 : hour};
            if constexpr (Kortlinje::time::format == 12U)
            {
                DisplayService::setPixel(DisplayService::Symbol::AM, local.tm_hour < 12);
                DisplayService::setPixel(DisplayService::Symbol::PM, local.tm_hour >= 12);
            }
            const KortlinjeFont font{};
            const TextHandler text{std::format("{:02d}:{:02d}", _hour, minute), font};
            text.draw(0, static_cast<int8_t>((Display::grid::rows - text.getHeight()) / 2U));
        }
    }
    const float rad{static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)};
    const float dx{cosf(rad)};
    const float dy{-sinf(rad)};
    const float maxComponent{std::max(fabsf(dx), fabsf(dy))};
    const float stepX{dx / maxComponent};
    const float stepY{dy / maxComponent};
    const int targetX{static_cast<int>(std::lroundf(xDec + stepX))};
    const int targetY{static_cast<int>(std::lroundf(yDec + stepY))};
    const uint8_t nextX{static_cast<uint8_t>(std::clamp(targetX, 0, static_cast<int>(Display::grid::maxX)))};
    const uint8_t nextY{static_cast<uint8_t>(std::clamp(targetY, 0, static_cast<int>(Display::grid::maxY)))};
    if (x == 0U && deg >= 90U && deg < 270U)
    {
        deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
    }
    if ((y == 0U && deg < 180U) || (y >= Display::grid::maxY && deg > 180U))
    {
        deg = 360U - deg; // Invert Y
    }
    if (nextX >= Display::grid::maxX && (deg < 90U || deg > 270U))
    {
        deg = static_cast<uint16_t>(random(150L, 211L));
    }
    else if ((nextX != x || nextY != y) && DisplayService::getPixel(nextX, nextY))
    {
        DisplayService::setPixel(nextX, nextY, false);
        if (nextX == x)
        {
            deg = 360U - deg; // Invert Y
        }
        else if (nextY == y)
        {
            deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
        }
        else
        {
            deg = deg >= 180U ? 540U - deg : 180U - deg;
        }
    }
    const float moveRad{static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)};
    const float moveDx{cosf(moveRad)};
    const float moveDy{-sinf(moveRad)};
    const float moveMaxComponent{std::max(fabsf(moveDx), fabsf(moveDy))};
    DisplayService::setPixel(x, y, false);
    xDec += moveDx / moveMaxComponent;
    yDec += moveDy / moveMaxComponent;
    xDec = std::clamp(xDec, .0F, static_cast<float>(Display::grid::maxX));
    yDec = std::clamp(yDec, .0F, static_cast<float>(Display::grid::maxY));
    x = static_cast<uint8_t>(std::clamp<int>(std::lroundf(xDec), 0, static_cast<int>(Display::grid::maxX)));
    y = static_cast<uint8_t>(std::clamp<int>(std::lroundf(yDec), 0, static_cast<int>(Display::grid::maxY)));
    DisplayService::setPixel(x, y, true);
    const float angleRad{atanf(std::max(.0F, static_cast<float>(Display::grid::maxX - 1U) - xDec) /
                               std::max(.1F, std::fabs(static_cast<float>(paddleY) - yDec)))};
    if (yDec < paddleY - 1U && angleRad < 1.0F && paddleY > 1U)
    {
        DisplayService::setPixel(Display::grid::maxX, paddleY + 1U, false);
        --paddleY;
        DisplayService::setPixel(Display::grid::maxX, paddleY - 1U, true);
    }
    else if (yDec > paddleY + 1U && angleRad < 1.0F && paddleY < Display::grid::maxY - 1U)
    {
        DisplayService::setPixel(Display::grid::maxX, paddleY - 1U, false);
        ++paddleY;
        DisplayService::setPixel(Display::grid::maxX, paddleY + 1U, true);
    }
    interval = static_cast<int64_t>(static_cast<float>(0b1U << 16U) / std::max(0.01F, moveMaxComponent));
}

int64_t __time_critical_func(BreakoutMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<BreakoutMode *>(user_data)};
    self->tick = true;
    return self->interval;
}
