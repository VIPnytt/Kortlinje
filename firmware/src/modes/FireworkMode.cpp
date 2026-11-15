#include "modes/FireworkMode.h"

#include "services/DisplayService.h"

FireworkMode::~FireworkMode() { stopAlarm(); }

void FireworkMode::begin()
{
    stage = 0U;
    rocketY = GRID_ROWS;
    radius = 0U;
}

void FireworkMode::handle()
{
    switch (stage)
    {
    case 1U:
        launching();
        break;
    case 2U:
        exploding();
        break;
    default:
        pad();
        break;
    }
}

void FireworkMode::end() { stopAlarm(); }

/**
 * @brief Initializes a rocket launch when the firework trigger is activated.
 *
 * Selects a random launch column, places the rocket below the display, and
 * sets the mode to the launching stage.
 */
void FireworkMode::pad()
{
    if (tick)
    {
        tick = false;
        rocketX = static_cast<uint8_t>(random(GRID_COLUMNS));
        rocketY = GRID_ROWS;
        stage = 1U;
    }
    startAlarm(static_cast<uint32_t>(random(static_cast<long>(0b1U << 8U))));
}

/**
 * @brief Moves the rocket upward and begins the explosion stage at a random height in the upper half of the display.
 */
void FireworkMode::launching()
{
    if (tick)
    {
        tick = false;
        if (rocketY < GRID_ROWS)
        {
            DisplayService::setPixel(rocketX, rocketY, false);
        }
        --rocketY;
        DisplayService::setPixel(rocketX, rocketY, true);
        if (rocketY <= random((static_cast<long>(GRID_ROWS) / 2L) + 1L))
        {
            radius = 0U;
            maxRadius = static_cast<uint8_t>(random(1L, (static_cast<long>(GRID_ROWS) / 2L) + 1L));
            stage = 2U;
        }
        startAlarm(0b1U << 7U);
    }
}

/**
 * @brief Advances the firework explosion, renders its expanding ellipse, and clears it once maximum radius is reached.
 */
void FireworkMode::exploding()
{
    if (tick)
    {
        tick = false;
        if (radius < maxRadius)
        {
            ++radius;
            DisplayService::drawEllipseSolid(rocketX, rocketY, radius, true);
            startAlarm(radius < maxRadius ? 0b1U << 8U
                                          : static_cast<uint32_t>(
                                                random(static_cast<long>(0b1U << 8U), static_cast<long>(0b1U << 11U))));
        }
        else
        {
            DisplayService::drawEllipseSolid(rocketX, rocketY, radius, false);
            stage = 0U;
        }
    }
}

int64_t __time_critical_func(FireworkMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<FireworkMode *>(user_data)};
    self->alarmId = 0L;
    self->tick = true;
    return 0LL;
}

void FireworkMode::startAlarm(uint32_t delayMs)
{
    stopAlarm();
    alarmId = add_alarm_in_ms(delayMs, &onAlarm, this, true);
}

void FireworkMode::stopAlarm()
{
    if (alarmId > 0)
    {
        cancel_alarm(alarmId);
        alarmId = 0;
    }
    tick = false;
}
