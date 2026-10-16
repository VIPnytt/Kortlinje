#include "modes/PingPongMode.h"

#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

void PingPongMode::begin()
{
    Display.fillFrame(false);
    paddleL.clear();
    paddleR.clear();
    const uint8_t _paddle{static_cast<uint8_t>(random(0, GRID_ROWS - 3U))};
    for (uint8_t idx{0U}; idx < 3U; ++idx)
    {
        paddleL.push_back(_paddle + idx);
        paddleR.push_back(_paddle + idx);
    }
    Display.drawLineVertical(0U, _paddle, _paddle + 2U, true);
    Display.drawLineVertical(GRID_COLUMNS - 1U, _paddle, _paddle + 2U, true);
    xDec = x = GRID_COLUMNS - 2U;
    yDec = y = _paddle + 1U;
    deg = static_cast<uint16_t>(random(150L, 211L)); // ±30°
    Display.setPixel(x, y, true);
}

void PingPongMode::handle()
{
    if (xDec <= 1 && deg >= 90U && deg < 270U)
    {
        // Left
        deg = (random(60L, 121L) + 270U) % 360U; // ±30°
    }
    else if (xDec >= GRID_COLUMNS - 2U && (deg < 90U || deg >= 270U))
    {
        // Right
        deg = static_cast<uint16_t>(random(150L, 211L)); // ±30°
    }
    if (xDec <= 0U || xDec >= GRID_COLUMNS - 1U)
    {
        // Left/right
        deg = deg >= 180U ? 540U - deg : 180U - deg; // Invert X
    }
    if (yDec <= 0U || yDec >= GRID_ROWS - 1U)
    {
        // Top/bottom
        deg = 360U - deg; // Invert Y
    }
    Display.setPixel(x, y, false);
    xDec += cosf(static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)) * speed;
    yDec -= sinf(static_cast<float>(deg) * static_cast<float>(DEG_TO_RAD)) * speed;
    x = lroundf(xDec);
    y = lroundf(yDec);
    Display.setPixel(x, y, true);
    const float lRad{atanf((xDec - 1U) / abs(paddleL[1U] - yDec))};
    const float rRad{atanf((GRID_COLUMNS - 2U - xDec) / abs(paddleR[1U] - yDec))};
    if (yDec > paddleL.back() && lRad < 1.3F && paddleL.back() < GRID_ROWS - 1U)
    {
        // Left: down
        Display.setPixel(0U, paddleL.front(), false);
        paddleL.pop_front();
        paddleL.push_back(paddleL.back() + 1U);
        Display.setPixel(0U, paddleL.back(), true);
    }
    else if (yDec < paddleL.front() && lRad < 1.3F && paddleL.front() != 0U)
    {
        // Left: up
        Display.setPixel(0U, paddleL.back(), false);
        paddleL.pop_back();
        paddleL.push_front(paddleL.front() - 1U);
        Display.setPixel(0U, paddleL.front(), true);
    }
    else if (yDec > paddleR.back() && rRad < 1.3F && paddleR.back() < GRID_ROWS - 1U)
    {
        // Right: down
        Display.setPixel(GRID_COLUMNS - 1U, paddleR.front(), false);
        paddleR.pop_front();
        paddleR.push_back(paddleR.back() + 1U);
        Display.setPixel(GRID_COLUMNS - 1U, paddleR.back(), true);
    }
    else if (yDec < paddleR.front() && rRad < 1.3F && paddleR.front() != 0U)
    {
        // Right: up
        Display.setPixel(GRID_COLUMNS - 1U, paddleR.back(), false);
        paddleR.pop_back();
        paddleR.push_front(paddleR.front() - 1U);
        Display.setPixel(GRID_COLUMNS - 1U, paddleR.front(), true);
    }
}
