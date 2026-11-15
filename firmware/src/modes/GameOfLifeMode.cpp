#include "modes/GameOfLifeMode.h"

#include "config/IkeaKortlinje.h"
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
    std::array<bool, GRID_COLUMNS * GRID_ROWS> grid{};
    for (size_t idx{0U}; idx < grid.size(); ++idx)
    {
        grid[idx] = DisplayService::getPixel(idx);
    }
    for (uint8_t idx{active}; idx < static_cast<uint8_t>(GRID_COLUMNS * GRID_ROWS / (0b1U << 4U)); ++idx)
    {
        grid[random(1L, GRID_COLUMNS - 1L) + (random(1L, GRID_ROWS - 1L) * GRID_COLUMNS)] = true;
    }
    active = 0U;
    for (uint8_t x{0U}; x < GRID_COLUMNS; ++x)
    {
        const uint8_t xStart{x > 0U ? static_cast<uint8_t>(x - 1U) : 0U};
        const uint8_t xEnd{static_cast<uint8_t>(std::min<uint8_t>(x + 1U, GRID_COLUMNS - 1U))};
        for (uint8_t y{0U}; y < GRID_ROWS; ++y)
        {
            const uint8_t yStart{y > 0U ? static_cast<uint8_t>(y - 1U) : 0U};
            const uint8_t yEnd{static_cast<uint8_t>(std::min<uint8_t>(y + 1U, GRID_ROWS - 1U))};
            uint8_t count{0U};
            for (uint8_t _x{xStart}; _x <= xEnd; ++_x)
            {
                for (uint8_t _y{yStart}; _y <= yEnd; ++_y)
                {
                    if ((_x != x || _y != y) && grid[_x + (_y * GRID_COLUMNS)])
                    {
                        ++count;
                    }
                }
            }
            const bool lit{grid[x + (y * GRID_COLUMNS)]};
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
