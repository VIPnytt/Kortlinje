#include "modes/EpochCounterMode.h"

#include "services/DisplayService.h"

void EpochCounterMode::begin() { DisplayService::fillFrame(false); }

/**
 * @brief Displays the current Unix epoch time in binary on the device grid.
 *
 * The display is refreshed when the epoch value changes, with 32 bits arranged
 * across 2 rows of four 4-bit nibbles using 1-by-4 vertical bars.
 */
void EpochCounterMode::handle()
{
    const time_t _epoch{time(nullptr)};
    if (_epoch != epoch)
    {
        epoch = _epoch;
        for (uint8_t idx{0U}; idx < 32U; ++idx)
        {
            const uint8_t column{static_cast<uint8_t>(idx % 16U)};
            const uint8_t y{static_cast<uint8_t>(Display::grid::maxY - 3U - ((idx / 16U) * 5U))};
            DisplayService::drawLineVertical(
                static_cast<uint8_t>(Display::grid::maxX - 1U - (column * 2U) - ((column / 4U) * 3U)),
                y,
                static_cast<uint8_t>(y + 3U),
                ((static_cast<std::make_unsigned_t<time_t>>(epoch) >> idx) & std::make_unsigned_t<time_t>{1ULL}) != 0U);
        }
    }
}
