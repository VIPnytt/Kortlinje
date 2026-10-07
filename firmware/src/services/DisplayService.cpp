#include "services/DisplayService.h"

#include <ranges>

void DisplayService::begin()
{
#ifdef PIN_LED
    pinMode(PIN_LED, PinMode::OUTPUT_2MA);
#endif // PIN_LED
    for (const pin_size_t pin : sclk)
    {
        pinMode(pin, PinMode::OUTPUT_2MA);
        digitalWrite(pin, PinStatus::HIGH);
    }
    for (const pin_size_t pin : din)
    {
        pinMode(pin, PinMode::OUTPUT_2MA);
        digitalWrite(pin, PinStatus::HIGH);
    }
    setBrightness(Brightness::DUTY16_1);
}

void DisplayService::drawEllipseOutline(float x, float y, float radius, bool lit)
{
    const float radiusSq{radius * radius};
    for (size_t _x{static_cast<size_t>(max(.0F, ceilf(x - radius)))};
         _x <= min(GRID_COLUMNS - 1U, static_cast<unsigned int>(floorf(x + radius)));
         ++_x)
    {
        const float xDistance{static_cast<float>(_x) - x};
        const float yDistance{sqrtf(max(.0F, radiusSq - (xDistance * xDistance)))};
        const int top{static_cast<int>(ceilf(y - yDistance))};
        for (int _y{max(0, top - 1)}; _y <= min(static_cast<int>(GRID_ROWS - 1U), top + 1); ++_y)
        {
            const float _yDistance{static_cast<float>(_y) - y};
            if (fabsf((xDistance * xDistance) + (_yDistance * _yDistance) - radiusSq) < radius)
            {
                frame[_x + (static_cast<size_t>(_y) * GRID_COLUMNS)] = lit;
            }
        }
        const int bottom{static_cast<int>(floorf(y + yDistance))};
        for (int _y{max(0, bottom - 1)}; _y <= min(static_cast<int>(GRID_ROWS - 1U), bottom + 1); ++_y)
        {
            const float _dy{static_cast<float>(_y) - y};
            if (fabsf((xDistance * xDistance) + (_dy * _dy) - radiusSq) < radius)
            {
                frame[_x + (static_cast<size_t>(_y) * GRID_COLUMNS)] = lit;
            }
        }
    }
    for (size_t _y{static_cast<size_t>(max(.0F, ceilf(y - radius)))};
         _y <= min(GRID_ROWS - 1U, static_cast<unsigned int>(floorf(y + radius)));
         ++_y)
    {
        const float yDistance{static_cast<float>(_y) - y};
        const float xDistance{sqrtf(max(.0F, radiusSq - (yDistance * yDistance)))};
        const int left{static_cast<int>(ceilf(x - xDistance))};
        for (int _x{max(0, left - 1)}; _x <= min(static_cast<int>(GRID_COLUMNS - 1U), left + 1); ++_x)
        {
            const float _xDistance{static_cast<float>(_x) - x};
            if (fabsf((_xDistance * _xDistance) + (yDistance * yDistance) - radiusSq) < radius)
            {
                frame[static_cast<size_t>(_x) + (_y * GRID_COLUMNS)] = lit;
            }
        }
        const int right{static_cast<int>(floorf(x + xDistance))};
        for (int _x{max(0, right - 1)}; _x <= min(static_cast<int>(GRID_COLUMNS - 1U), right + 1); ++_x)
        {
            const float _xDistance{static_cast<float>(_x) - x};
            if (fabsf((_xDistance * _xDistance) + (yDistance * yDistance) - radiusSq) < radius)
            {
                frame[static_cast<size_t>(_x) + (_y * GRID_COLUMNS)] = lit;
            }
        }
    }
    render = true;
}

void DisplayService::drawEllipseSolid(float x, float y, float radius, bool lit)
{
    const float radiusSq{radius * radius};
    for (size_t _y{static_cast<size_t>(max(.0F, ceilf(y - radius)))};
         _y <= min(GRID_ROWS - 1U, static_cast<unsigned int>(floorf(y + radius)));
         ++_y)
    {
        const float yDistance{static_cast<float>(_y) - y};
        const float xDistance{sqrtf(max(.0F, radiusSq - (yDistance * yDistance)))};
        const size_t minX{static_cast<size_t>(max(.0F, ceilf(x - xDistance)))};
        std::ranges::fill(std::span{frame}.subspan(
                              static_cast<size_t>(minX + (_y * GRID_COLUMNS)),
                              min<size_t>(GRID_COLUMNS - 1U, static_cast<size_t>(floorf(x + xDistance))) - minX + 1U),
                          lit);
    }
    render = true;
}

void DisplayService::drawLineHorizontal(uint8_t xMin, size_t columns, uint8_t y, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(xMin + (y * GRID_COLUMNS), columns), lit);
    render = true;
}

void DisplayService::drawLineVertical(uint8_t x, uint8_t yMin, uint8_t yMax, bool lit)
{
    for (size_t idx{static_cast<size_t>(x + (yMin * GRID_COLUMNS))}; idx <= x + (yMax * GRID_COLUMNS);
         idx += GRID_COLUMNS)
    {
        frame[idx] = lit;
    }
    render = true;
}

void DisplayService::drawRectangleOutline(size_t minX, size_t columns, size_t minY, size_t maxY, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(minX + (minY * GRID_COLUMNS), columns), lit);
    std::ranges::fill(std::span{frame}.subspan(minX + (maxY * GRID_COLUMNS), columns), lit);
    const size_t maxX{static_cast<size_t>(minX + columns - 1U)};
    for (size_t y{static_cast<size_t>(minY + 1U)}; y < maxY; ++y)
    {
        frame[minX + (y * GRID_COLUMNS)] = lit;
        frame[maxX + (y * GRID_COLUMNS)] = lit;
    }
    render = true;
}

void DisplayService::drawRectangleSolid(uint8_t minX, uint8_t columns, uint8_t minY, uint8_t maxY, bool lit)
{
    for (size_t y{minY}; y <= maxY; ++y)
    {
        std::ranges::fill(std::span{frame}.subspan(minX + (y * GRID_COLUMNS), columns), lit);
    }
    render = true;
}

void DisplayService::fillColumn(uint8_t x, bool lit)
{
    for (size_t idx{static_cast<size_t>(x)}; idx < GRID_COLUMNS * GRID_ROWS; idx += GRID_COLUMNS)
    {
        frame[idx] = lit;
    }
    render = true;
}

void DisplayService::fillFrame(bool lit)
{
    frame.fill(lit);
    render = true;
}

void DisplayService::fillRow(uint8_t y, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(y * GRID_COLUMNS, GRID_COLUMNS), lit);
    render = true;
}

void DisplayService::fillRows(size_t minY, size_t rows, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(minY * GRID_COLUMNS, rows * GRID_COLUMNS), lit);
    render = true;
}

void DisplayService::flush()
{
    if (!render)
    {
        return;
    }
    render = false;
    constexpr size_t chips{pixels.size() >> 7U};
    std::array<std::array<uint8_t, 16U>, chips> nextBuffer{};
    for (size_t idx{0U}; idx < pixels.size(); ++idx)
    {
        if (frame[idx])
        {
            nextBuffer[pixels[idx] >> 7U][(pixels[idx] & 0x7FU) >> 3U] |=
                static_cast<uint8_t>(1U << (pixels[idx] & 7U));
        }
    }
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        uint16_t dirtyGrids{0U};
        for (size_t grid{0U}; grid < 16U; ++grid)
        {
            if (nextBuffer[chip][grid] != buffer[chip][grid])
            {
                dirtyGrids |= (1U << grid);
            }
        }
        if (dirtyGrids == 0U)
        {
            continue;
        }
        buffer[chip] = nextBuffer[chip];
        bool active{false};
        for (uint8_t grid{0U}; grid < 16U; ++grid)
        {
            if (dirtyGrids & (1U << grid))
            {
                if (!active)
                {
                    vkStart(chip);
                    vkSet(chip, 0xC0U | grid);
                    active = true;
                }
                vkSet(chip, buffer[chip][grid]);
            }
            else if (active)
            {
                vkEnd(chip);
                active = false;
            }
        }
        if (active)
        {
            vkEnd(chip);
        }
    }
}

DisplayService::Brightness DisplayService::getBrightness() const { return brightness; }

void DisplayService::getFrame(std::span<bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> _frame) const
{
    std::ranges::copy(frame, _frame.begin());
}

bool DisplayService::getPixel(size_t idx) const { return frame[idx]; }

bool DisplayService::getPixel(uint8_t x, uint8_t y) const { return frame[static_cast<size_t>(x + (y * GRID_COLUMNS))]; }

bool DisplayService::getPixel(Symbol symbol) const
{
    return frame[static_cast<size_t>(symbol) + static_cast<size_t>(GRID_COLUMNS * GRID_ROWS)];
}
bool DisplayService::getPower() const { return power; }

void DisplayService::invertFrame()
{
    for (bool &lit : frame)
    {
        lit = !lit;
    }
    render = true;
}

void DisplayService::setBrightness(Brightness _brightness)
{
    if (power && _brightness == brightness)
    {
        return;
    }
    for (size_t chip{0U}; chip < pixels.size() >> 7U; ++chip)
    {
        vkStart(chip);
        vkSet(chip, static_cast<uint8_t>(_brightness));
        vkEnd(chip);
    }
    brightness = _brightness;
    power = true;
}

void DisplayService::setFrame(std::span<const bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> _frame)
{
    std::ranges::copy(_frame, frame.begin());
    render = true;
}

void DisplayService::setPixel(size_t idx, bool lit)
{
    frame[idx] = lit;
    render = true;
}

void DisplayService::setPixel(uint8_t x, uint8_t y, bool lit)
{
    frame[static_cast<size_t>(x + (y * GRID_COLUMNS))] = lit;
    render = true;
}

void DisplayService::setPixel(Symbol symbol, bool lit)
{
    frame[static_cast<size_t>(symbol) + static_cast<size_t>(GRID_COLUMNS * GRID_ROWS)] = lit;
    render = true;
#ifdef PIN_LED
    if (symbol == Symbol::STATUS)
    {
        digitalWrite(PIN_LED, lit);
    }
#endif // PIN_LED
}

void DisplayService::setPower(bool _power)
{
    if (power == _power)
    {
        return;
    }
    if (_power)
    {
        setBrightness(brightness);
    }
    else
    {
        for (size_t chip{0U}; chip < pixels.size() >> 7U; ++chip)
        {
            vkStart(chip);
            vkSet(chip, 0b1000'0000U);
            vkEnd(chip);
        }
    }
}

void DisplayService::vkStart(size_t chip)
{
    digitalWrite(din[chip], PinStatus::HIGH);
    digitalWrite(sclk[chip], PinStatus::HIGH);
    busy_wait_at_least_cycles(cycles);
    digitalWrite(din[chip], PinStatus::LOW);
    busy_wait_at_least_cycles(cycles);
    digitalWrite(sclk[chip], PinStatus::LOW);
}

void DisplayService::vkEnd(size_t chip)
{
    digitalWrite(din[chip], PinStatus::LOW);
    busy_wait_at_least_cycles(cycles);
    digitalWrite(sclk[chip], PinStatus::HIGH);
    busy_wait_at_least_cycles(cycles);
    digitalWrite(din[chip], PinStatus::HIGH);
}

void DisplayService::vkSet(size_t chip, uint8_t byte)
{
    for (uint8_t idx{0U}; idx < 8U; idx++, byte >>= 1U)
    {
        digitalWrite(sclk[chip], PinStatus::LOW);
        digitalWrite(din[chip], byte & 1U);
        busy_wait_at_least_cycles(cycles);
        digitalWrite(sclk[chip], PinStatus::HIGH);
        busy_wait_at_least_cycles(cycles);
    }
    digitalWrite(sclk[chip], PinStatus::LOW);
}

DisplayService &DisplayService::getInstance()
{
    static DisplayService instance;
    return instance;
}

DisplayService &Display{DisplayService::getInstance()};
