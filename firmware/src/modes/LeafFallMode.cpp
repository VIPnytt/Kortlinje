#include "modes/LeafFallMode.h"

LeafFallMode::~LeafFallMode() { cancel_repeating_timer(&timer); }

/**
 * @brief Initializes leaf coordinates and renders the initial frame.
 */
void LeafFallMode::begin()
{
    DisplayService::fillFrame(false);
    for (Leaf &leaf : leaves)
    {
        do
        {
            leaf.x = static_cast<uint8_t>(random(static_cast<long>(Display::grid::columns)));
            leaf.y = static_cast<uint8_t>(random(static_cast<long>(Display::grid::rows)));
        } while (DisplayService::getPixel(leaf.x, leaf.y));
        DisplayService::setPixel(leaf.x, leaf.y, true);
    }
    add_repeating_timer_us(-static_cast<int64_t>(0b1U << 16U), &onTimer, this, &timer);
}

/**
 * @brief Probabilistically advances leaves on each timer tick.
 */
void LeafFallMode::handle()
{
    if (!tick)
    {
        return;
    }
    tick = false;
    for (Leaf &leaf : leaves)
    {
        if (random(static_cast<long>(0b1U << 3U)) == 0L)
        {
            if (leaf.y >= Display::grid::maxY)
            {
                DisplayService::setPixel(leaf.x, leaf.y, false);
                leaf.y = 0U;
                do
                {
                    leaf.x = static_cast<uint8_t>(random(static_cast<long>(Display::grid::columns)));
                } while (DisplayService::getPixel(leaf.x, leaf.y));
                DisplayService::setPixel(leaf.x, leaf.y, true);
            }
            else
            {
                uint8_t nextX{leaf.x};
                uint8_t nextY{leaf.y};
                const long dir{random(static_cast<long>(0b1U << 2U))};
                if (dir == 0 && leaf.x != 0U)
                {
                    --nextX;
                }
                else if (dir == 1 && leaf.x < Display::grid::maxX)
                {
                    ++nextX;
                }
                else
                {
                    ++nextY;
                }
                if (!DisplayService::getPixel(nextX, nextY))
                {
                    DisplayService::setPixel(leaf.x, leaf.y, false);
                    leaf.x = nextX;
                    leaf.y = nextY;
                    DisplayService::setPixel(leaf.x, leaf.y, true);
                }
            }
        }
    }
}

bool __time_critical_func(LeafFallMode::onTimer)(repeating_timer *rt)
{
    static_cast<LeafFallMode *>(rt->user_data)->tick = true;
    return true;
}
