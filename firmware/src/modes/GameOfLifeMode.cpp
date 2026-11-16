#include "modes/GameOfLifeMode.h"

#include "services/DisplayService.h"

#include <algorithm>
#include <array>

GameOfLifeMode::~GameOfLifeMode() { cancel_repeating_timer(&timer); }

void GameOfLifeMode::begin() { add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 8U), &onTimer, this, &timer); }

/**
 * @brief Advances the Conway's Game of Life simulation with self-regulating interior seeding.
 */
void GameOfLifeMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    std::array<bool, Display::grid::pixels> grid{};
    for (size_t idx{0U}; idx < grid.size(); ++idx)
    {
        grid[idx] = DisplayService::getPixel(idx);
    }
    for (uint8_t idx{active}; idx < static_cast<uint8_t>(Display::grid::pixels / (0b1U << 4U)); ++idx)
    {
        grid[random(1L, static_cast<long>(Display::grid::maxX)) +
             (random(1L, static_cast<long>(Display::grid::maxY)) * Display::grid::columns)] = true;
    }
    active = 0U;
    for (uint8_t x{0U}; x < Display::grid::columns; ++x)
    {
        const uint8_t xStart{static_cast<uint8_t>(x > 0U ? x - 1U : 0U)};
        const uint8_t xEnd{static_cast<uint8_t>(std::min<uint8_t>(x + 1U, Display::grid::maxX))};
        for (uint8_t y{0U}; y < Display::grid::rows; ++y)
        {
            const uint8_t yStart{static_cast<uint8_t>(y > 0U ? y - 1U : 0U)};
            const uint8_t yEnd{static_cast<uint8_t>(std::min<uint8_t>(y + 1U, Display::grid::maxY))};
            uint8_t count{0U};
            for (uint8_t _x{xStart}; _x <= xEnd; ++_x)
            {
                for (uint8_t _y{yStart}; _y <= yEnd; ++_y)
                {
                    if ((_x != x || _y != y) && grid[_x + (_y * Display::grid::columns)])
                    {
                        ++count;
                    }
                }
            }
            const bool lit{grid[x + (y * Display::grid::columns)]};
            const bool next{count == 3U || (lit && count == 2U)};
            DisplayService::setPixel(x, y, next);
            if (!lit && next && active != 0xFFU)
            {
                ++active;
            }
        }
    }
}

bool __time_critical_func(GameOfLifeMode::onTimer)(repeating_timer *rt)
{
    static_cast<GameOfLifeMode *>(rt->user_data)->tick = true;
    return true;
}
