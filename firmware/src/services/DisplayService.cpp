#include "services/DisplayService.h"

#include "extensions/WebUiExtension.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "pio/display.pio.h"
#include "services/DeviceService.h"

#include <ranges>

DisplayService::DisplayService() : ServiceModule(name)
{
    pio_add_program_at_offset(pio0, &display_program, 0U);
    for (uint chip{0U}; chip < chips; ++chip)
    {
        pio_sm_claim(pio0, chip);
        dma_channel_claim(chip);
    }
}

void DisplayService::begin()
{
#ifdef PIN_LED
    pinMode(PIN_LED, PinMode::OUTPUT_2MA);
#endif // PIN_LED
    ConfigHandler::get<Brightness>(name, "brightness", brightness);
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        pio_sm_config pio{display_program_get_default_config(0U)};
        sm_config_set_sideset_pins(&pio, sclk[chip]);
        sm_config_set_out_pins(&pio, sda[chip], 1U);
        sm_config_set_set_pins(&pio, sda[chip], 1U);
        sm_config_set_out_shift(&pio, true, false, 32U);
        sm_config_set_fifo_join(&pio, pio_fifo_join::PIO_FIFO_JOIN_TX);
        sm_config_set_clkdiv_int_frac8(&pio, static_cast<uint32_t>(F_CPU / 4'000'000L), 0U);
        pio_gpio_init(pio0, sclk[chip]);
        pio_gpio_init(pio0, sda[chip]);
        gpio_set_drive_strength(sclk[chip], gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
        gpio_set_drive_strength(sda[chip], gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
        pio_sm_set_consecutive_pindirs(pio0, static_cast<uint>(chip), sclk[chip], 1U, true);
        pio_sm_set_consecutive_pindirs(pio0, static_cast<uint>(chip), sda[chip], 1U, true);
        pio_sm_set_pins_with_mask(pio0,
                                  static_cast<uint>(chip),
                                  (0b1U << sda[chip]) | (0b1U << sclk[chip]),
                                  (0b1U << sda[chip]) | (0b1U << sclk[chip]));
        pio_sm_init(pio0, static_cast<uint>(chip), 0U, &pio);
        pio_sm_set_enabled(pio0, static_cast<uint>(chip), true);
        dma_channel_config_t dma{dma_channel_get_default_config(static_cast<uint>(chip))};
        channel_config_set_transfer_data_size(&dma, dma_channel_transfer_size::DMA_SIZE_32);
        channel_config_set_read_increment(&dma, true);
        channel_config_set_write_increment(&dma, false);
        channel_config_set_dreq(&dma, pio_get_dreq(pio0, static_cast<uint>(chip), true));
        txBuffer[chip][1U] = 0xC0U << 1U;
        txBuffer[chip][18U] = 1U;
        txBuffer[chip][20U] = static_cast<uint32_t>(brightness) << 1U;
        txBuffer[chip][21U] = 1U;
        dma_channel_configure(
            static_cast<uint>(chip), &dma, &pio0->txf[static_cast<uint>(chip)], txBuffer[chip].data(), 22U, true);
    }
    KortlinjeFont font{};
    TextHandler("Hej ♥", font).draw();
}

void DisplayService::handle()
{
    if (pending)
    {
        transmit();
        pending = false;
    }
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
    std::ranges::fill(std::span{frame}.first((GRID_COLUMNS * GRID_ROWS) + 2U), lit);
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
    std::array<std::array<uint8_t, 16U>, chips> buffer{};
    for (size_t idx{0U}; idx < pixels.size(); ++idx)
    {
        if (frame[idx])
        {
            buffer[pixels[idx] >> 7U][(pixels[idx] & 0x7FU) >> 3U] |= static_cast<uint8_t>(1U << (pixels[idx] & 7U));
        }
    }
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        uint16_t dirtyGrids{0U};
        for (size_t grid{0U}; grid < 16U; ++grid)
        {
            if (buffer[chip][grid] != vkBuffer[chip][grid])
            {
                dirtyGrids |= (1U << grid);
            }
        }
        if (dirtyGrids == 0U)
        {
            continue;
        }
        vkBuffer[chip] = buffer[chip];
        size_t idx{0U};
        bool active{false};
        while (dma_channel_is_busy(static_cast<uint>(chip)))
        {
            tight_loop_contents();
        }
        for (uint8_t grid{0U}; grid < 16U; ++grid)
        {
            if (dirtyGrids & (1U << grid))
            {
                if (!active)
                {
                    txBuffer[chip][idx++] = 0U;
                    txBuffer[chip][idx++] = (0xC0U | grid) << 1U;
                    active = true;
                }
                txBuffer[chip][idx++] = static_cast<uint32_t>(vkBuffer[chip][grid]) << 1U;
            }
            else if (active)
            {
                txBuffer[chip][idx++] = 1U;
                active = false;
            }
        }
        if (active)
        {
            txBuffer[chip][idx++] = 1U;
        }
        if (idx != 0U)
        {
            dma_channel_transfer_from_buffer_now(
                static_cast<uint>(chip), txBuffer[chip].data(), static_cast<uint32_t>(idx));
        }
    }
    WebUiExtension::render = true;
}

DisplayService::Brightness DisplayService::getBrightness() const { return brightness; }

std::span<const bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> DisplayService::getFrame() const { return frame; }

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

void DisplayService::onReceive(JsonObjectConst payload, std::string_view source)
{
    // Brightness
    if (payload["brightness"].is<uint8_t>())
    {
        setBrightness(static_cast<Brightness>((payload["brightness"].as<uint8_t>()) +
                                              static_cast<uint8_t>(Brightness::DUTY16_1) - 1U));
    }
    // Power
    if (payload["power"].is<bool>())
    {
        setPower(payload["power"].as<bool>());
    }
}

void DisplayService::setBrightness(Brightness _brightness)
{
    if (power && _brightness == brightness)
    {
        return;
    }
    brightness = _brightness;
    power = true;
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        while (dma_channel_is_busy(static_cast<uint>(chip)))
        {
            tight_loop_contents();
        }
        txBuffer[chip][0U] = 0U;
        txBuffer[chip][1U] = static_cast<uint32_t>(brightness) << 1U;
        txBuffer[chip][2U] = 1U;
        dma_channel_transfer_from_buffer_now(static_cast<uint>(chip), txBuffer[chip].data(), 3U);
    }
    ConfigHandler::set<Brightness>(name, "brightness", brightness);
    pending = true;
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
    if (_power == power)
    {
        return;
    }
    if (_power)
    {
        setBrightness(brightness);
    }
    else
    {
        power = false;
        for (size_t chip{0U}; chip < chips; ++chip)
        {
            while (dma_channel_is_busy(static_cast<uint>(chip)))
            {
                tight_loop_contents();
            }
            txBuffer[chip][0U] = 0U;
            txBuffer[chip][1U] = 0b1000'0000U << 1U;
            txBuffer[chip][2U] = 1U;
            dma_channel_transfer_from_buffer_now(static_cast<uint>(chip), txBuffer[chip].data(), 3U);
        }
        pending = true;
    }
}

void DisplayService::transmit()
{
    JsonDocument doc{};
    doc["brightness"].set(static_cast<uint8_t>(brightness) + 1U - static_cast<uint8_t>(Brightness::DUTY16_1));
    doc["power"].set(power);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}

DisplayService &DisplayService::getInstance()
{
    static DisplayService instance;
    return instance;
}

DisplayService &Display{DisplayService::getInstance()};
