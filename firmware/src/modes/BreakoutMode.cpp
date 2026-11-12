#include "modes/BreakoutMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/BitmapHandler.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"

#include <algorithm>
#include <cmath>

BreakoutMode::~BreakoutMode() { stopAlarm(); }

void BreakoutMode::begin()
{
    DisplayService::fillFrame(false);
    for (uint8_t column{0U}; column <= GRID_COLUMNS - 4U; ++column)
    {
        DisplayService::fillColumn(column, true);
    }
    paddleY = static_cast<uint8_t>(random(1L, GRID_ROWS - 1U));
    DisplayService::drawLineVertical(GRID_COLUMNS - 1U, paddleY - 1U, paddleY + 1U, true);
    deg = static_cast<uint16_t>(random(150L, 211L)); // ±30°
    xDec = x = GRID_COLUMNS - 2U;
    yDec = y = paddleY;
    DisplayService::setPixel(x, y, true);
    startAlarm();
}

void BreakoutMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    if (x == GRID_COLUMNS - 2U)
    {
        now = time(nullptr);
        if (localtime_r(&now, &local) != nullptr && (minute != local.tm_min || hour != local.tm_hour))
        {
            pending = false;
            hour = local.tm_hour;
            minute = local.tm_min;
#if CLOCK_12H
            const int hour{(local.tm_hour + 11) % 12 + 1};
            DisplayService::setPixel(DisplayService::Symbol::AM, local.tm_hour < 12);
            DisplayService::setPixel(DisplayService::Symbol::PM, local.tm_hour >= 12);
#endif // CLOCK_12H
            uint8_t cursorX{0U};
            const uint8_t hh1{static_cast<uint8_t>(hour / 10)};
            BitmapHandler(KortlinjeFont::digitZero_digitNine[hh1]).draw(cursorX, (GRID_ROWS / 2U) - 4U);
            cursorX += hh1 == 1U ? 7U : 8U;
            const uint8_t hh2{static_cast<uint8_t>(hour % 10)};
            BitmapHandler(KortlinjeFont::digitZero_digitNine[hh2]).draw(cursorX, (GRID_ROWS / 2U) - 4U);
            cursorX += hh2 == 1U ? 7U : 8U;
            BitmapHandler(KortlinjeFont::colon).draw(cursorX, (GRID_ROWS / 2U) - 3U);
            cursorX += 3U;
            const uint8_t mm1{static_cast<uint8_t>(minute / 10)};
            BitmapHandler(KortlinjeFont::digitZero_digitNine[mm1]).draw(cursorX, (GRID_ROWS / 2U) - 4U);
            cursorX += mm1 == 1U ? 7U : 8U;
            const uint8_t mm2{static_cast<uint8_t>(minute % 10)};
            BitmapHandler(KortlinjeFont::digitZero_digitNine[mm2]).draw(cursorX, (GRID_ROWS / 2U) - 4U);
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
    const uint8_t nextX{static_cast<uint8_t>(std::clamp(targetX, 0, static_cast<int>(GRID_COLUMNS - 1U)))};
    const uint8_t nextY{static_cast<uint8_t>(std::clamp(targetY, 0, static_cast<int>(GRID_ROWS - 1U)))};
    if (x == 0U && deg >= 90U && deg < 270U)
    {
        deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
    }
    if ((y == 0U && deg < 180U) || (y >= GRID_ROWS - 1U && deg > 180U))
    {
        deg = 360U - deg; // Invert Y
    }
    if (nextX >= GRID_COLUMNS - 1U && (deg < 90U || deg > 270U))
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
    xDec = std::clamp(xDec, .0F, static_cast<float>(GRID_COLUMNS - 1U));
    yDec = std::clamp(yDec, .0F, static_cast<float>(GRID_ROWS - 1U));
    x = static_cast<uint8_t>(std::clamp<int>(std::lroundf(xDec), 0, GRID_COLUMNS - 1));
    y = static_cast<uint8_t>(std::clamp<int>(std::lroundf(yDec), 0, GRID_ROWS - 1));
    DisplayService::setPixel(x, y, true);
    const float angleRad{atanf(std::max(.0F, static_cast<float>(GRID_COLUMNS - 2U) - xDec) /
                               std::max(.1F, std::fabs(static_cast<float>(paddleY) - yDec)))};
    if (yDec < paddleY - 1U && angleRad < 1.0F && paddleY > 1U)
    {
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleY + 1U, false);
        --paddleY;
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleY - 1U, true);
    }
    else if (yDec > paddleY + 1U && angleRad < 1.0F && paddleY < GRID_ROWS - 2U)
    {
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleY - 1U, false);
        ++paddleY;
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleY + 1U, true);
    }
    startAlarm();
}

void BreakoutMode::end() { stopAlarm(); }

int64_t __time_critical_func(BreakoutMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<BreakoutMode *>(user_data)};
    self->tick = true;
    self->alarmId = 0L;
    return 0LL;
}

void BreakoutMode::startAlarm()
{
    stopAlarm();
    const float rad{static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)};
    alarmId = add_alarm_in_us(static_cast<uint64_t>(static_cast<float>(0b1U << 16U) /
                                                    std::max(0.01F, std::max(fabsf(cosf(rad)), fabsf(sinf(rad))))),
                              &onAlarm,
                              this,
                              true);
}

void BreakoutMode::stopAlarm()
{
    if (alarmId > 0L)
    {
        cancel_alarm(alarmId);
        alarmId = 0L;
    }
    tick = false;
}
