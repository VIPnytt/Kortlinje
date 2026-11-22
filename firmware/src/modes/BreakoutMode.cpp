#include "modes/BreakoutMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"

#include <algorithm>
#include <cmath>
#include <numbers>

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
    const uint8_t nextX{
        static_cast<uint8_t>(std::clamp(static_cast<int>(x) + stepX, 0, static_cast<int>(Display::grid::maxX)))};
    const uint8_t nextY{static_cast<uint8_t>(
        std::clamp(static_cast<int>(std::lroundf(yDec + stepY)), 0, static_cast<int>(Display::grid::maxY)))};
    if (x == 0U && stepX < 0)
    {
        stepX = 1;
    }
    if ((y == 0U && stepY < .0F) || (y >= Display::grid::maxY && stepY > .0F))
    {
        stepY = -stepY;
    }
    if (nextX == Display::grid::maxX && stepX > 0)
    {
        stepX = -1;
        stepY = (static_cast<float>(random(101L)) * 1e-2F) -
                (static_cast<float>(y) * (1.0F / static_cast<float>(Display::grid::maxY)));
        interval = static_cast<int64_t>(static_cast<float>(0b1U << 16U) * sqrtf(1.0F + (stepY * stepY)));
    }
    else if ((nextX != x || nextY != y) && DisplayService::getPixel(nextX, nextY))
    {
        DisplayService::setPixel(nextX, nextY, false);
        if (nextX == x)
        {
            stepY = -stepY;
        }
        else
        {
            stepX = -stepX;
        }
    }
    DisplayService::setPixel(x, y, false);
    x = static_cast<uint8_t>(std::clamp(static_cast<int>(x) + stepX, 0, static_cast<int>(Display::grid::maxX - 1U)));
    yDec = std::clamp(yDec + stepY, .0F, static_cast<float>(Display::grid::maxY));
    y = static_cast<uint8_t>(std::lroundf(yDec));
    DisplayService::setPixel(x, y, true);
    const float distX{static_cast<float>((Display::grid::maxX - 1U) - x)};
    const float distY{fabsf(static_cast<float>(paddleY) - yDec)};
    if (distX < distY * std::numbers::sqrt3_v<float>)
    {
        if (paddleY > 1U && yDec < static_cast<float>(paddleY - 1U))
        {
            DisplayService::setPixel(Display::grid::maxX, paddleY + 1U, false);
            --paddleY;
            DisplayService::setPixel(Display::grid::maxX, paddleY - 1U, true);
        }
        else if (paddleY < Display::grid::maxY - 1U && yDec > static_cast<float>(paddleY + 1U))
        {
            DisplayService::setPixel(Display::grid::maxX, paddleY - 1U, false);
            ++paddleY;
            DisplayService::setPixel(Display::grid::maxX, paddleY + 1U, true);
        }
    }
}

int64_t __time_critical_func(BreakoutMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<BreakoutMode *>(user_data)};
    self->tick = true;
    return self->interval;
}
