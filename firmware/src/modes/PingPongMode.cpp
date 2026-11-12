#include "modes/PingPongMode.h"

#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <algorithm>
#include <cmath>

PingPongMode::~PingPongMode() { stopAlarm(); }

void PingPongMode::begin()
{
    DisplayService::fillFrame(false);
    paddleL = paddleR = static_cast<uint8_t>(random(1, GRID_ROWS - 1U));
    DisplayService::drawLineVertical(0U, paddleL - 1U, paddleL + 1U, true);
    DisplayService::drawLineVertical(GRID_COLUMNS - 1U, paddleR - 1U, paddleR + 1U, true);
    xDec = x = GRID_COLUMNS - 2U;
    yDec = y = paddleR;
    deg = static_cast<uint16_t>(random(150L, 211L)); // ±30°
    DisplayService::setPixel(x, y, true);
    startAlarm();
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
        // Left paddle reflection
        deg = (random(60L, 121L) + 270U) % 360U; // ±30°
    }
    else if (xDec >= static_cast<float>(GRID_COLUMNS - 2U) && (deg < 90U || deg >= 270U))
    {
        // Right paddle reflection
        deg = static_cast<uint16_t>(random(150L, 211L)); // ±30°
    }
    if (xDec <= .0F || xDec >= static_cast<float>(GRID_COLUMNS - 1U))
    {
        deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
        xDec = std::clamp(xDec, .0F, static_cast<float>(GRID_COLUMNS - 1U));
    }
    if (yDec <= .0F || yDec >= static_cast<float>(GRID_ROWS - 1U))
    {
        deg = 360U - deg; // Invert Y
        yDec = std::clamp(yDec, .0F, static_cast<float>(GRID_ROWS - 1U));
    }
    const float rad{static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)};
    const float dx{cosf(rad)};
    const float dy{-sinf(rad)};
    const float maxComponent{std::max(fabsf(dx), fabsf(dy))};
    DisplayService::setPixel(x, y, false);
    xDec += dx / maxComponent;
    yDec += dy / maxComponent;
    x = static_cast<uint8_t>(std::clamp<int>(lroundf(xDec), 0, GRID_COLUMNS - 1));
    y = static_cast<uint8_t>(std::clamp<int>(lroundf(yDec), 0, GRID_ROWS - 1));
    DisplayService::setPixel(x, y, true);
    const float lRad{atanf((xDec - 1.0F) / std::max(.1F, fabsf(static_cast<float>(paddleL) - yDec)))};
    const float rRad{atanf((static_cast<float>(GRID_COLUMNS - 2U) - xDec) /
                           std::max(.1F, fabsf(static_cast<float>(paddleR) - yDec)))};
    if (yDec > paddleL + 1U && lRad < 1.3F && paddleL < GRID_ROWS - 2U)
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
    if (yDec > paddleR + 1U && rRad < 1.3F && paddleR < GRID_ROWS - 2U)
    {
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleR - 1U, false);
        ++paddleR;
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleR + 1U, true);
    }
    else if (yDec < paddleR - 1U && rRad < 1.3F && paddleR > 1U)
    {
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleR + 1U, false);
        --paddleR;
        DisplayService::setPixel(GRID_COLUMNS - 1U, paddleR - 1U, true);
    }
    startAlarm();
}

void PingPongMode::end() { stopAlarm(); }

int64_t __time_critical_func(PingPongMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    PingPongMode *self{static_cast<PingPongMode *>(user_data)};
    self->tick = true;
    self->alarmId = 0L;
    return 0LL;
}

void PingPongMode::startAlarm()
{
    stopAlarm();
    const float rad{static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)};
    alarmId = add_alarm_in_us(static_cast<uint64_t>(static_cast<float>(0b1U << 15U) /
                                                    std::max(.01F, std::max(fabsf(cosf(rad)), fabsf(sinf(rad))))),
                              &onAlarm,
                              this,
                              true);
}

void PingPongMode::stopAlarm()
{
    if (alarmId > 0L)
    {
        cancel_alarm(alarmId);
        alarmId = 0L;
    }
    tick = false;
}
