#include "services/DisplayService.h"

#include "constants/HomeAssistant.h"
#include "extensions/MirrorExtension.h"
#include "extensions/WebUiExtension.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "pio/display.pio.h"
#include "services/DeviceService.h"

#include <bit>
#include <ranges>

void DisplayService::begin()
{
#ifdef PIN_LED
    pinMode(PIN_LED, PinMode::OUTPUT_2MA);
#endif // PIN_LED
#ifdef VK1640_SCLK_1M25
    pio_add_program_at_offset(pio0, &display_1m25_program, 0U);
#else
    pio_add_program_at_offset(pio0, &display_program, 0U);
#endif // VK1640_SCLK_1M25
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        pio_sm_claim(pio0, chip);
        dma_channel_claim(chip);
#ifdef VK1640_SCLK_1M25
        pio_sm_config pio{display_1m25_program_get_default_config(0U)};
#else
        pio_sm_config pio{display_program_get_default_config(0U)};
#endif // VK1640_SCLK_1M25
        sm_config_set_sideset_pins(&pio, sclk[chip]);
        sm_config_set_out_pins(&pio, sda[chip], 1U);
        sm_config_set_set_pins(&pio, sda[chip], 1U);
        sm_config_set_out_shift(&pio, true, false, 32U);
        sm_config_set_fifo_join(&pio, pio_fifo_join::PIO_FIFO_JOIN_TX);
#ifdef VK1640_SCLK_1M25
        sm_config_set_clkdiv_int_frac8(&pio, static_cast<uint32_t>(F_CPU) / 5'000'000UL, 0U);
#else
        sm_config_set_clkdiv_int_frac8(&pio, static_cast<uint32_t>(F_CPU) / 4'000'000UL, 0U);
#endif // VK1640_SCLK_1M25
        pio_gpio_init(pio0, sclk[chip]);
        pio_gpio_init(pio0, sda[chip]);
        gpio_set_drive_strength(sclk[chip], gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
        gpio_set_drive_strength(sda[chip], gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
        pio_sm_set_consecutive_pindirs(pio0, chip, sclk[chip], 1U, true);
        pio_sm_set_consecutive_pindirs(pio0, chip, sda[chip], 1U, true);
        const uint32_t mask{(0b1UL << sda[chip]) | (0b1UL << sclk[chip])};
        pio_sm_set_pins_with_mask(pio0, chip, mask, mask);
#ifdef VK1640_SCLK_1M25
        pio_sm_init(pio0, chip, display_1m25_offset_idle, &pio);
#else
        pio_sm_init(pio0, chip, display_offset_idle, &pio);
#endif // VK1640_SCLK_1M25
        pio_sm_set_enabled(pio0, chip, true);
        dma_channel_config_t dma{dma_channel_get_default_config(chip)};
        channel_config_set_transfer_data_size(&dma, dma_channel_transfer_size::DMA_SIZE_32);
        channel_config_set_read_increment(&dma, true);
        channel_config_set_write_increment(&dma, false);
        channel_config_set_dreq(&dma, pio_get_dreq(pio0, chip, true));
        txBuffer[chip][1U] = 0xC0UL << 1U;
        txBuffer[chip][18U] = 1UL;
        dma_channel_configure(chip, &dma, &pio0->txf[chip], txBuffer[chip].data(), txBuffer[chip].size(), true);
    }
    ConfigHandler::get(name, "brightness", brightness);
    const KortlinjeFont font{};
    TextHandler("Hej! ♥", font).draw();
    setPower(true);
    flush();
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
         _x <= min(Display::grid::maxX, static_cast<unsigned int>(floorf(x + radius)));
         ++_x)
    {
        const float xDistance{static_cast<float>(_x) - x};
        const float yDistance{sqrtf(max(.0F, radiusSq - (xDistance * xDistance)))};
        const int top{static_cast<int>(ceilf(y - yDistance))};
        for (int _y{max(0, top - 1)}; _y <= min(static_cast<int>(Display::grid::maxY), top + 1); ++_y)
        {
            const float _yDistance{static_cast<float>(_y) - y};
            if (fabsf((xDistance * xDistance) + (_yDistance * _yDistance) - radiusSq) < radius)
            {
                frame[_x + (static_cast<size_t>(_y) * Display::grid::columns)] = lit;
            }
        }
        const int bottom{static_cast<int>(floorf(y + yDistance))};
        for (int _y{max(0, bottom - 1)}; _y <= min(static_cast<int>(Display::grid::maxY), bottom + 1); ++_y)
        {
            const float _dy{static_cast<float>(_y) - y};
            if (fabsf((xDistance * xDistance) + (_dy * _dy) - radiusSq) < radius)
            {
                frame[_x + (static_cast<size_t>(_y) * Display::grid::columns)] = lit;
            }
        }
    }
    for (size_t _y{static_cast<size_t>(max(.0F, ceilf(y - radius)))};
         _y <= min(Display::grid::maxY, static_cast<unsigned int>(floorf(y + radius)));
         ++_y)
    {
        const float yDistance{static_cast<float>(_y) - y};
        const float xDistance{sqrtf(max(.0F, radiusSq - (yDistance * yDistance)))};
        const int left{static_cast<int>(ceilf(x - xDistance))};
        for (int _x{max(0, left - 1)}; _x <= min(static_cast<int>(Display::grid::maxX), left + 1); ++_x)
        {
            const float _xDistance{static_cast<float>(_x) - x};
            if (fabsf((_xDistance * _xDistance) + (yDistance * yDistance) - radiusSq) < radius)
            {
                frame[static_cast<size_t>(_x) + (_y * Display::grid::columns)] = lit;
            }
        }
        const int right{static_cast<int>(floorf(x + xDistance))};
        for (int _x{max(0, right - 1)}; _x <= min(static_cast<int>(Display::grid::maxX), right + 1); ++_x)
        {
            const float _xDistance{static_cast<float>(_x) - x};
            if (fabsf((_xDistance * _xDistance) + (yDistance * yDistance) - radiusSq) < radius)
            {
                frame[static_cast<size_t>(_x) + (_y * Display::grid::columns)] = lit;
            }
        }
    }
    render = true;
}

void DisplayService::drawEllipseSolid(float x, float y, float radius, bool lit)
{
    const float radiusSq{radius * radius};
    for (size_t _y{static_cast<size_t>(max(.0F, ceilf(y - radius)))};
         _y <= min(Display::grid::maxY, static_cast<unsigned int>(floorf(y + radius)));
         ++_y)
    {
        const float yDistance{static_cast<float>(_y) - y};
        const float xDistance{sqrtf(max(.0F, radiusSq - (yDistance * yDistance)))};
        const size_t minX{static_cast<size_t>(max(.0F, ceilf(x - xDistance)))};
        std::ranges::fill(std::span{frame}.subspan(
                              static_cast<size_t>(minX + (_y * Display::grid::columns)),
                              min<size_t>(Display::grid::maxX, static_cast<size_t>(floorf(x + xDistance))) - minX + 1U),
                          lit);
    }
    render = true;
}

void DisplayService::drawLineHorizontal(uint8_t xMin, size_t columns, uint8_t y, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(xMin + (y * Display::grid::columns), columns), lit);
    render = true;
}

void DisplayService::drawLineVertical(uint8_t x, uint8_t yMin, uint8_t yMax, bool lit)
{
    for (size_t idx{static_cast<size_t>(x + (yMin * Display::grid::columns))};
         idx <= x + (yMax * Display::grid::columns);
         idx += Display::grid::columns)
    {
        frame[idx] = lit;
    }
    render = true;
}

void DisplayService::drawRectangleOutline(size_t minX, size_t columns, size_t minY, size_t maxY, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(minX + (minY * Display::grid::columns), columns), lit);
    std::ranges::fill(std::span{frame}.subspan(minX + (maxY * Display::grid::columns), columns), lit);
    const size_t maxX{static_cast<size_t>(minX + columns - 1U)};
    for (size_t y{static_cast<size_t>(minY + 1U)}; y < maxY; ++y)
    {
        frame[minX + (y * Display::grid::columns)] = lit;
        frame[maxX + (y * Display::grid::columns)] = lit;
    }
    render = true;
}

void DisplayService::drawRectangleSolid(uint8_t minX, uint8_t columns, uint8_t minY, uint8_t maxY, bool lit)
{
    for (size_t y{minY}; y <= maxY; ++y)
    {
        std::ranges::fill(std::span{frame}.subspan(minX + (y * Display::grid::columns), columns), lit);
    }
    render = true;
}

void DisplayService::fillColumn(uint8_t x, bool lit)
{
    for (size_t idx{static_cast<size_t>(x)}; idx < Display::grid::pixels; idx += Display::grid::columns)
    {
        frame[idx] = lit;
    }
    render = true;
}

void DisplayService::fillFrame(bool lit)
{
    std::ranges::fill(std::span{frame}.first(lit ? Display::grid::pixels : Display::grid::pixels + 2U), lit);
    render = true;
}

void DisplayService::fillRow(uint8_t y, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(y * Display::grid::columns, Display::grid::columns), lit);
    render = true;
}

void DisplayService::fillRows(size_t minY, size_t rows, bool lit)
{
    std::ranges::fill(std::span{frame}.subspan(minY * Display::grid::columns, rows * Display::grid::columns), lit);
    render = true;
}

void DisplayService::flush()
{
    if (!render)
    {
        return;
    }
    render = false;
    static constexpr std::array<uint16_t, leds> pixels{
        0x0U,
        0x1U,
        0x2U,
        0x3U,
        0x4U,
        0x5U,
        0x6U,
        0x7U,
        0x48U,
        0x49U,
        0x4AU,
        0x4BU,
        0x4CU,
        0x4DU,
        0x80U,
        0x81U,
        0x82U,
        0x83U,
        0x84U,
        0x85U,
        0x86U,
        0x87U,
        0xC8U,
        0xC9U,
        0xCAU,
        0xCBU,
        0xCCU,
        0xCDU,
        0x100U,
        0x101U,
        0x102U,
        0x103U,
        0x104U,
        0x105U,
        0x106U,
        0x107U,
        0x148U,
        0x149U,
        0x14AU,
        0x14BU,
        0x14CU,
        0x14DU,
        0x8U,
        0x9U,
        0xAU,
        0xBU,
        0xCU,
        0xDU,
        0xEU,
        0xFU,
        0x4EU,
        0x4FU,
        0x50U,
        0x51U,
        0x52U,
        0x53U,
        0x88U,
        0x89U,
        0x8AU,
        0x8BU,
        0x8CU,
        0x8DU,
        0x8EU,
        0x8FU,
        0xCEU,
        0xCFU,
        0xD0U,
        0xD1U,
        0xD2U,
        0xD3U,
        0x108U,
        0x109U,
        0x10AU,
        0x10BU,
        0x10CU,
        0x10DU,
        0x10EU,
        0x10FU,
        0x14EU,
        0x14FU,
        0x150U,
        0x151U,
        0x152U,
        0x153U,
        0x10U,
        0x11U,
        0x12U,
        0x13U,
        0x14U,
        0x15U,
        0x16U,
        0x17U,
        0x54U,
        0x55U,
        0x56U,
        0x57U,
        0x58U,
        0x59U,
        0x90U,
        0x91U,
        0x92U,
        0x93U,
        0x94U,
        0x95U,
        0x96U,
        0x97U,
        0xD4U,
        0xD5U,
        0xD6U,
        0xD7U,
        0xD8U,
        0xD9U,
        0x110U,
        0x111U,
        0x112U,
        0x113U,
        0x114U,
        0x115U,
        0x116U,
        0x117U,
        0x154U,
        0x155U,
        0x156U,
        0x157U,
        0x158U,
        0x159U,
        0x18U,
        0x19U,
        0x1AU,
        0x1BU,
        0x1CU,
        0x1DU,
        0x1EU,
        0x1FU,
        0x5AU,
        0x5BU,
        0x5CU,
        0x5DU,
        0x5EU,
        0x5FU,
        0x98U,
        0x99U,
        0x9AU,
        0x9BU,
        0x9CU,
        0x9DU,
        0x9EU,
        0x9FU,
        0xDAU,
        0xDBU,
        0xDCU,
        0xDDU,
        0xDEU,
        0xDFU,
        0x118U,
        0x119U,
        0x11AU,
        0x11BU,
        0x11CU,
        0x11DU,
        0x11EU,
        0x11FU,
        0x15AU,
        0x15BU,
        0x15CU,
        0x15DU,
        0x15EU,
        0x15FU,
        0x20U,
        0x21U,
        0x22U,
        0x23U,
        0x24U,
        0x25U,
        0x26U,
        0x27U,
        0x60U,
        0x61U,
        0x62U,
        0x63U,
        0x64U,
        0x65U,
        0xA0U,
        0xA1U,
        0xA2U,
        0xA3U,
        0xA4U,
        0xA5U,
        0xA6U,
        0xA7U,
        0xE0U,
        0xE1U,
        0xE2U,
        0xE3U,
        0xE4U,
        0xE5U,
        0x120U,
        0x121U,
        0x122U,
        0x123U,
        0x124U,
        0x125U,
        0x126U,
        0x127U,
        0x160U,
        0x161U,
        0x162U,
        0x163U,
        0x164U,
        0x165U,
        0x28U,
        0x29U,
        0x2AU,
        0x2BU,
        0x2CU,
        0x2DU,
        0x2EU,
        0x2FU,
        0x66U,
        0x67U,
        0x68U,
        0x69U,
        0x6AU,
        0x6BU,
        0xA8U,
        0xA9U,
        0xAAU,
        0xABU,
        0xACU,
        0xADU,
        0xAEU,
        0xAFU,
        0xE6U,
        0xE7U,
        0xE8U,
        0xE9U,
        0xEAU,
        0xEBU,
        0x128U,
        0x129U,
        0x12AU,
        0x12BU,
        0x12CU,
        0x12DU,
        0x12EU,
        0x12FU,
        0x166U,
        0x167U,
        0x168U,
        0x169U,
        0x16AU,
        0x16BU,
        0x30U,
        0x31U,
        0x32U,
        0x33U,
        0x34U,
        0x35U,
        0x36U,
        0x37U,
        0x6CU,
        0x6DU,
        0x6EU,
        0x6FU,
        0x70U,
        0x71U,
        0xB0U,
        0xB1U,
        0xB2U,
        0xB3U,
        0xB4U,
        0xB5U,
        0xB6U,
        0xB7U,
        0xECU,
        0xEDU,
        0xEEU,
        0xEFU,
        0xF0U,
        0xF1U,
        0x130U,
        0x131U,
        0x132U,
        0x133U,
        0x134U,
        0x135U,
        0x136U,
        0x137U,
        0x16CU,
        0x16DU,
        0x16EU,
        0x16FU,
        0x170U,
        0x171U,
        0x38U,
        0x39U,
        0x3AU,
        0x3BU,
        0x3CU,
        0x3DU,
        0x3EU,
        0x3FU,
        0x72U,
        0x73U,
        0x74U,
        0x75U,
        0x76U,
        0x77U,
        0xB8U,
        0xB9U,
        0xBAU,
        0xBBU,
        0xBCU,
        0xBDU,
        0xBEU,
        0xBFU,
        0xF2U,
        0xF3U,
        0xF4U,
        0xF5U,
        0xF6U,
        0xF7U,
        0x138U,
        0x139U,
        0x13AU,
        0x13BU,
        0x13CU,
        0x13DU,
        0x13EU,
        0x13FU,
        0x172U,
        0x173U,
        0x174U,
        0x175U,
        0x176U,
        0x177U,
        0x40U,
        0x41U,
        0x42U,
        0x43U,
        0x44U,
        0x45U,
        0x46U,
        0x47U,
        0x78U,
        0x79U,
        0x7AU,
        0x7BU,
        0x7CU,
        0x7DU,
        0xC0U,
        0xC1U,
        0xC2U,
        0xC3U,
        0xC4U,
        0xC5U,
        0xC6U,
        0xC7U,
        0xF8U,
        0xF9U,
        0xFAU,
        0xFBU,
        0xFCU,
        0xFDU,
        0x140U,
        0x141U,
        0x142U,
        0x143U,
        0x144U,
        0x145U,
        0x146U,
        0x147U,
        0x178U,
        0x179U,
        0x17AU,
        0x17BU,
        0x17CU,
        0x17DU,
        0x7EU,
        0x7FU,
        0xFEU,
        0x17EU,
        0xFFU,
        0x17FU,
    };
    alignas(4U) std::array<std::array<uint8_t, 16U>, chips> buffer{};
    for (size_t idx{0U}; idx < pixels.size(); ++idx)
    {
        if (frame[idx])
        {
            buffer[pixels[idx] >> 7U][(pixels[idx] & 0x7FU) >> 3U] |= static_cast<uint8_t>(0b1U << (pixels[idx] & 7U));
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
        bool active{false};
        size_t idx{0U};
        alignas(4U) std::array<uint32_t, 19U> staging{};
        for (size_t grid{0U}; grid < 16U; ++grid)
        {
            if (dirtyGrids & (1U << grid))
            {
                if (!active)
                {
                    staging[idx++] = 0UL;
                    staging[idx++] = (0xC0UL | grid) << 1U;
                    active = true;
                }
                staging[idx++] = static_cast<uint32_t>(buffer[chip][grid]) << 1U;
            }
            else if (active)
            {
                const uint16_t remaining{static_cast<uint16_t>(dirtyGrids >> grid)};
                if (std::countr_zero(remaining) <= 3)
                {
                    staging[idx++] = static_cast<uint32_t>(buffer[chip][grid]) << 1U;
                }
                else
                {
                    staging[idx++] = 1UL;
                    active = false;
                    if (remaining == 0U)
                    {
                        break;
                    }
                }
            }
        }
        if (active)
        {
            staging[idx++] = 1UL;
        }
        while (dma_channel_is_busy(chip))
        {
            tight_loop_contents();
        }
        std::ranges::copy(std::span{staging}.first(idx), txBuffer[chip].begin());
        dma_channel_transfer_from_buffer_now(chip, txBuffer[chip].data(), idx);
        vkBuffer[chip] = buffer[chip];
    }
    MirrorExtension::render = true;
    WebUiExtension::render = true;
}

DisplayService::Brightness DisplayService::getBrightness() { return brightness; }

std::span<const bool, Display::grid::pixels> DisplayService::getFrame()
{
    return std::span{frame}.first<Display::grid::pixels>();
}

bool DisplayService::getPixel(size_t idx) { return frame[idx]; }

bool DisplayService::getPixel(uint8_t x, uint8_t y)
{
    return frame[static_cast<size_t>(x + (y * Display::grid::columns))];
}

bool DisplayService::getPixel(Symbol symbol) { return frame[static_cast<size_t>(symbol) + Display::grid::pixels]; }

bool DisplayService::getPower() { return power; }

void DisplayService::invertFrame()
{
    for (bool &lit : std::span{frame}.first(Display::grid::pixels))
    {
        lit = !lit;
    }
    render = true;
}

/**
 * @brief Adds the Home Assistant light component configuration to a discovery document.
 *
 * @param discovery Discovery document to update.
 * @param topic Base MQTT topic for the component state.
 * @param unique Prefix used to construct the component's unique identifier.
 */
void DisplayService::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    const std::string id{std::string(name).append("_light")};
    const std::string topicDisplay{std::string(topic).append(name)};
    const std::string topicModes{std::string(topic).append(ModesService::name)};
    JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
    component[HomeAssistant::Abbreviations::brightness_command_template].set(R"({"brightness":{{value}}})");
    component[HomeAssistant::Abbreviations::brightness_command_topic].set(topicDisplay + "/set");
    component[HomeAssistant::Abbreviations::brightness_scale].set(8U);
    component[HomeAssistant::Abbreviations::brightness_state_topic].set(topicDisplay);
    component[HomeAssistant::Abbreviations::brightness_value_template].set("{{value_json.brightness}}");
    component[HomeAssistant::Abbreviations::command_topic].set(topicDisplay + "/set");
    component[HomeAssistant::Abbreviations::effect_command_template].set(R"({"mode":"{{value}}"})");
    component[HomeAssistant::Abbreviations::effect_command_topic].set(topicModes + "/set");
    JsonArray effectList{component[HomeAssistant::Abbreviations::effect_list].to<JsonArray>()};
    for (const std::string_view _mode : ModesService::names)
    {
        effectList.add(_mode);
    }
    component[HomeAssistant::Abbreviations::effect_state_topic].set(topicModes);
    component[HomeAssistant::Abbreviations::effect_value_template].set("{{value_json.mode}}");
    component[HomeAssistant::Abbreviations::icon].set("mdi:clock-digital");
    component[HomeAssistant::Abbreviations::name].set("");
    component[HomeAssistant::Abbreviations::on_command_type].set("brightness");
    component[HomeAssistant::Abbreviations::payload_off].set(R"({"power":false})");
    component[HomeAssistant::Abbreviations::payload_on].set(R"({"power":true})");
    component[HomeAssistant::Abbreviations::platform].set("light");
    component[HomeAssistant::Abbreviations::state_topic].set(topicDisplay);
    component[HomeAssistant::Abbreviations::state_value_template].set(R"({"power":{{value_json.power|lower}}})");
    component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
}

void DisplayService::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["brightness"].is<uint8_t>())
    {
        setBrightness(static_cast<Brightness>(static_cast<uint8_t>(payload["brightness"].as<uint8_t>() - 1U)));
    }
    if (payload["power"].is<bool>())
    {
        setPower(payload["power"].as<bool>());
    }
}

void DisplayService::setBrightness(Brightness _brightness)
{
    if (_brightness == brightness)
    {
        if (!power)
        {
            setPower(true);
        }
        return;
    }
    power = true;
    brightness = _brightness;
    const uint32_t command{(0b1000'1000UL | static_cast<uint32_t>(brightness)) << 1U};
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        while (dma_channel_is_busy(chip))
        {
            tight_loop_contents();
        }
        txBuffer[chip][1U] = command;
        txBuffer[chip][2U] = 1UL;
        dma_channel_transfer_from_buffer_now(chip, txBuffer[chip].data(), 3UL);
    }
    ConfigHandler::set(name, "brightness", brightness);
    pending = true;
}

void DisplayService::setFrame(std::span<const bool, Display::grid::pixels> _frame)
{
    std::ranges::copy(_frame, frame.begin());
    render = true;
}

void DisplayService::setFrame(std::span<const uint8_t, Display::grid::bytes> _frame)
{
    for (size_t idx{0U}; idx < Display::grid::pixels; ++idx)
    {
        frame[idx] = (_frame[idx >> 3U] & (0b1U << (idx & 7U))) != 0U;
    }
    render = true;
}

void DisplayService::setPixel(size_t idx, bool lit)
{
    frame[idx] = lit;
    render = true;
}

void DisplayService::setPixel(uint8_t x, uint8_t y, bool lit)
{
    frame[static_cast<size_t>(x + (y * Display::grid::columns))] = lit;
    render = true;
}

void DisplayService::setPixel(Symbol symbol, bool lit)
{
    const size_t idx{static_cast<size_t>(symbol) + Display::grid::pixels};
    if (frame[idx] == lit)
    {
        return;
    }
    frame[idx] = lit;
    render = true;
#ifdef PIN_LED
    if (symbol == Symbol::STATUS)
    {
        digitalWrite(PIN_LED, lit ? PinStatus::HIGH : PinStatus::LOW);
    }
#endif // PIN_LED
}

void DisplayService::setPower(bool enable)
{
    if (enable == power)
    {
        return;
    }
    power = enable;
    const uint32_t command{power ? (0b1000'1000UL | static_cast<uint32_t>(brightness)) << 1U : 0b1000'0000UL << 1U};
    for (size_t chip{0U}; chip < chips; ++chip)
    {
        while (dma_channel_is_busy(chip))
        {
            tight_loop_contents();
        }
        txBuffer[chip][1U] = command;
        txBuffer[chip][2U] = 1UL;
        dma_channel_transfer_from_buffer_now(chip, txBuffer[chip].data(), 3UL);
    }
    pending = true;
}

void DisplayService::transmit()
{
    JsonDocument doc{};
    doc["brightness"].set(static_cast<uint8_t>(brightness) + 1U);
    doc["power"].set(power);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
