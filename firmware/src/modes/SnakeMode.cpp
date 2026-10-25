#include "modes/SnakeMode.h"

#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <algorithm>
#include <array>

SnakeMode::~SnakeMode() { stopTimer(); }

void SnakeMode::begin()
{
    Display.fillFrame(false);
    snakeClear();
    target = static_cast<size_t>(random(0L, static_cast<long>(GRID_COLUMNS * GRID_ROWS)));
    stage = Stage::READY;
    startTimer();
}

void SnakeMode::handle()
{
    if (tick)
    {
        tick = false;
        switch (stage)
        {
        case Stage::READY:
            idle();
            break;
        case Stage::MOVE:
            move();
            break;
        case Stage::DEATH:
            blink();
            break;
        case Stage::REMOVE:
            clean();
            break;
        }
    }
}

void SnakeMode::end() { stopTimer(); }

void SnakeMode::idle()
{
    snakeReset(static_cast<size_t>(random(0L, static_cast<long>(GRID_COLUMNS * GRID_ROWS))));
    Display.setPixel(snake[head], true);
    setTarget();
    stage = Stage::MOVE;
}

std::optional<size_t> SnakeMode::findStepPath() const
{
    const uint8_t yMin{0U};
    std::array<size_t, GRID_COLUMNS * GRID_ROWS> from{};
    std::array<size_t, GRID_COLUMNS * GRID_ROWS> frontier{};
    std::array<bool, GRID_COLUMNS * GRID_ROWS> visited{};
    size_t frontierHead{0U};
    size_t frontierTail{0U};
    frontier[frontierTail++] = snake[head];
    from[snake[head]] = snake[head];
    visited[snake[head]] = true;
    while (frontierHead < frontierTail)
    {
        const size_t current{frontier[frontierHead]};
        ++frontierHead;
        if (current == target)
        {
            size_t step{target};
            while (from[step] != snake[head])
            {
                step = from[step];
            }
            return std::optional<size_t>{step};
        }
        std::array<size_t, 4U> neighbors{};
        size_t count{0U};
        if (current % GRID_COLUMNS != 0U)
        {
            neighbors[count++] = current - 1U;
        }
        if (current % GRID_COLUMNS < GRID_COLUMNS - 1U)
        {
            neighbors[count++] = current + 1U;
        }
        if (current / GRID_COLUMNS > yMin)
        {
            neighbors[count++] = current - GRID_COLUMNS;
        }
        if (current / GRID_COLUMNS < GRID_ROWS - 1U)
        {
            neighbors[count++] = current + GRID_COLUMNS;
        }
        for (size_t idx{0U}; idx < count; ++idx)
        {
            if (visited[neighbors[idx]] || occupied[neighbors[idx]])
            {
                continue;
            }
            visited[neighbors[idx]] = true;
            from[neighbors[idx]] = current;
            frontier[frontierTail] = neighbors[idx];
            ++frontierTail;
        }
    }
    return findStepAvailable();
}

std::optional<size_t> SnakeMode::findStepAvailable() const
{
    std::array<size_t, 4U> available{};
    size_t count{0U};
    if (snake[head] % GRID_COLUMNS > 0U)
    {
        available[count++] = snake[head] - 1U;
    }
    if (snake[head] % GRID_COLUMNS < GRID_COLUMNS - 1U)
    {
        available[count++] = snake[head] + 1U;
    }
    if (snake[head] / GRID_COLUMNS != 0U)
    {
        available[count++] = snake[head] - GRID_COLUMNS;
    }
    if (snake[head] / GRID_COLUMNS < GRID_ROWS - 1U)
    {
        available[count++] = snake[head] + GRID_COLUMNS;
    }
    std::optional<size_t> best{};
    size_t bestDistance{SIZE_MAX};
    for (size_t idx{0U}; idx < count; ++idx)
    {
        if (occupied[available[idx]])
        {
            continue;
        }
        const size_t distance{static_cast<size_t>(std::abs(static_cast<int>(available[idx] % GRID_COLUMNS) -
                                                           static_cast<int>(target % GRID_COLUMNS))) +
                              static_cast<size_t>(std::abs(static_cast<int>(available[idx] / GRID_COLUMNS) -
                                                           static_cast<int>(target / GRID_COLUMNS)))};
        if (distance < bestDistance)
        {
            best = available[idx];
            bestDistance = distance;
        }
    }
    return best;
}

void SnakeMode::move()
{
    const std::optional<size_t> step{findStepPath()};
    if (step.has_value())
    {
        if (!snakePushBack(step.value()))
        {
            setDead();
        }
        else if (snake[head] == target)
        {
            Display.setPixel(target, true);
            setTarget();
        }
        else
        {
            for (size_t idx{0U}; idx < length; ++idx)
            {
                Display.setPixel(snakeAt(idx), true);
            }
            Display.setPixel(snakePopFront(), false);
        }
    }
    else
    {
        setDead();
    }
}

void SnakeMode::blink()
{
    const bool lit{(blinkCount & 0b1U) != 0U};
    for (size_t idx{0U}; idx < length; ++idx)
    {
        Display.setPixel(snakeAt(idx), lit);
    }
    if (++blinkCount >= 6U)
    {
        stage = Stage::REMOVE;
    }
}

void SnakeMode::clean()
{
    if (length != 0U)
    {
        Display.setPixel(snakePopFront(), false);
    }
    else
    {
        Display.setPixel(target, false);
        stage = Stage::READY;
    }
}

bool __time_critical_func(SnakeMode::onTimer)(repeating_timer *rt)
{
    tick = true;
    return true;
}

void SnakeMode::snakeReset(size_t start)
{
    snakeClear();
    snake[0U] = start;
    occupied[start] = true;
    head = 0U;
    length = 1U;
}

void SnakeMode::snakeClear()
{
    occupied.fill(false);
    head = 0U;
    length = 0U;
}

bool SnakeMode::snakePushBack(size_t pixel)
{
    if (length >= GRID_COLUMNS * GRID_ROWS)
    {
        return false;
    }
    head = (head + 1U) % (GRID_COLUMNS * GRID_ROWS);
    snake[head] = pixel;
    occupied[pixel] = true;
    ++length;
    return true;
}

size_t SnakeMode::snakePopFront()
{
    const size_t tail{((GRID_COLUMNS * GRID_ROWS) + head - (length - 1U)) % (GRID_COLUMNS * GRID_ROWS)};
    occupied[snake[tail]] = false;
    --length;
    if (length == 0U)
    {
        head = 0U;
    }
    return snake[tail];
}

size_t SnakeMode::snakeAt(size_t index) const
{
    const size_t tail{((GRID_COLUMNS * GRID_ROWS) + head - (length - 1U)) % (GRID_COLUMNS * GRID_ROWS)};
    return snake[(tail + index) % (GRID_COLUMNS * GRID_ROWS)];
}

void SnakeMode::setDead()
{
    blinkCount = 0U;
    stage = Stage::DEATH;
}

void SnakeMode::setTarget()
{
    std::array<size_t, GRID_COLUMNS * GRID_ROWS> available{};
    size_t count{0U};
    for (size_t idx{0U}; idx < available.size(); ++idx)
    {
        if (!occupied[idx])
        {
            available[count++] = idx;
        }
    }
    if (count == 0U)
    {
        setDead();
        return;
    }
    target = available[static_cast<size_t>(random(static_cast<long>(count)))];
    Display.setPixel(target, true);
}

void SnakeMode::startTimer()
{
    if (!timerRunning)
    {
        timerRunning = add_repeating_timer_ms(-(0b1L << 7U), &onTimer, nullptr, &timer);
    }
}

void SnakeMode::stopTimer()
{
    if (timerRunning)
    {
        cancel_repeating_timer(&timer);
        timerRunning = false;
        tick = false;
    }
}
