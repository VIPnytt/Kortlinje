#include "modes/EqualizerMode.h"

#include "services/DisplayService.h"

void EqualizerMode::begin()
{
    for (uint8_t x{width}; x < GRID_COLUMNS; x += width + 1U)
    {
        for (uint8_t y{0U}; y < GRID_ROWS; ++y)
        {
            Display.setPixel(x, y, false);
        }
    }
}

void EqualizerMode::handle()
{
    if (millis() - lastMillis > (0b1U << 4U))
    {
        lastMillis = millis();
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
