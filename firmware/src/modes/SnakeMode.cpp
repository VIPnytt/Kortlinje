#include "modes/SnakeMode.h"

#include "services/DeviceService.h"

#include <algorithm>
#include <array>

SnakeMode::~SnakeMode() { cancel_repeating_timer(&timer); }

void SnakeMode::begin()
{
    DisplayService::fillFrame(false);
    add_repeating_timer_us(-static_cast<int64_t>(0b1U << 17U), &onTimer, this, &timer);
}

void SnakeMode::handle()
{
    if (!tick)
    {
        return;
    }
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

void SnakeMode::idle()
{
    snakeReset(static_cast<size_t>(random(static_cast<long>(Display::grid::pixels))));
    DisplayService::setPixel(snake[head], true);
    setTarget();
    stage = Stage::MOVE;
}

std::optional<size_t> SnakeMode::findStepPath() const
{
    const uint8_t yMin{0U};
    std::array<size_t, Display::grid::pixels> from{};
    std::array<size_t, Display::grid::pixels> frontier{};
    std::array<bool, Display::grid::pixels> visited{};
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
        if (current % Display::grid::columns != 0U)
        {
            neighbors[count++] = current - 1U;
        }
        if (current % Display::grid::columns < Display::grid::maxX)
        {
            neighbors[count++] = current + 1U;
        }
        if (current / Display::grid::columns > yMin)
        {
            neighbors[count++] = current - Display::grid::columns;
        }
        if (current / Display::grid::columns < Display::grid::maxY)
        {
            neighbors[count++] = current + Display::grid::columns;
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
    if (snake[head] % Display::grid::columns != 0U)
    {
        available[count++] = snake[head] - 1U;
    }
    if (snake[head] % Display::grid::columns < Display::grid::maxX)
    {
        available[count++] = snake[head] + 1U;
    }
    if (snake[head] / Display::grid::columns != 0U)
    {
        available[count++] = snake[head] - Display::grid::columns;
    }
    if (snake[head] / Display::grid::columns < Display::grid::maxY)
    {
        available[count++] = snake[head] + Display::grid::columns;
    }
    std::optional<size_t> best{};
    size_t bestDistance{SIZE_MAX};
    for (size_t idx{0U}; idx < count; ++idx)
    {
        if (occupied[available[idx]])
        {
            continue;
        }
        const size_t distance{static_cast<size_t>(std::abs(static_cast<int>(available[idx] % Display::grid::columns) -
                                                           static_cast<int>(target % Display::grid::columns))) +
                              static_cast<size_t>(std::abs(static_cast<int>(available[idx] / Display::grid::columns) -
                                                           static_cast<int>(target / Display::grid::columns)))};
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
            DisplayService::setPixel(target, true);
            setTarget();
        }
        else
        {
            for (size_t idx{0U}; idx < length; ++idx)
            {
                DisplayService::setPixel(snakeAt(idx), true);
            }
            DisplayService::setPixel(snakePopFront(), false);
        }
    }
    else
    {
        setDead();
    }
}

void SnakeMode::blink()
{
    const bool lit{(blinkCount & 0b10U) != 0U};
    for (size_t idx{0U}; idx < length; ++idx)
    {
        DisplayService::setPixel(snakeAt(idx), lit);
    }
    if (++blinkCount >= 0b1U << 4U)
    {
        stage = Stage::REMOVE;
    }
}

void SnakeMode::clean()
{
    if (length != 0U)
    {
        DisplayService::setPixel(snakePopFront(), false);
    }
    else
    {
        DisplayService::setPixel(target, false);
        stage = Stage::READY;
    }
}

bool __time_critical_func(SnakeMode::onTimer)(repeating_timer *rt)
{
    static_cast<SnakeMode *>(rt->user_data)->tick = true;
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
    if (length >= Display::grid::pixels)
    {
        return false;
    }
    head = (head + 1U) % Display::grid::pixels;
    snake[head] = pixel;
    occupied[pixel] = true;
    ++length;
    return true;
}

size_t SnakeMode::snakePopFront()
{
    const size_t tail{(Display::grid::pixels + head - (length - 1U)) % Display::grid::pixels};
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
    const size_t tail{(Display::grid::pixels + head - (length - 1U)) % Display::grid::pixels};
    return snake[(tail + index) % Display::grid::pixels];
}

void SnakeMode::setDead()
{
    blinkCount = 0U;
    stage = Stage::DEATH;
}

void SnakeMode::setTarget()
{
    std::array<size_t, Display::grid::pixels> available{};
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
    DisplayService::setPixel(target, true);
}
