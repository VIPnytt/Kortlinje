#include "extensions/Scd4xExtension.h"

#include "constants/HomeAssistant.h"
#include "constants/secrets.h"
#include "modes/Co2Mode.h"
#include "services/DeviceService.h"

#include <array>

namespace
{
constexpr int scl{static_cast<int>(Kortlinje::pin::co2_scl)};
constexpr int sda{static_cast<int>(Kortlinje::pin::co2_sda)};
constexpr bool enabled{scl >= 0 && sda >= 0};

static_assert(!enabled || (sda % 2 == 0 && scl == sda + 1), "co2_sda and co2_scl must be an I2C SDA/SCL pin pair");
} // namespace

void Scd4xExtension::begin()
{
    if constexpr (enabled)
    {
        TwoWire &wire{getWire()};
        wire.setSDA(static_cast<pin_size_t>(sda));
        wire.setSCL(static_cast<pin_size_t>(scl));
        wire.begin();
        resume();
    }
}

void Scd4xExtension::handle()
{
    if (state == State::absent || static_cast<int32_t>(millis() - deadline) < 0L)
    {
        return;
    }
    if (state == State::stopping)
    {
        std::array<uint16_t, 1U> word{};
        if (read(0x202FU, word))
        {
            switch (word[0] >> 12U)
            {
            case 0x0U:
                variant = "SCD40";
                break;
            case 0x1U:
                variant = "SCD41";
                break;
            case 0x5U:
                variant = "SCD43";
                break;
            default:
                break;
            }
        }
        std::array<uint16_t, 3U> serial{};
        if (read(0x3682U, serial))
        {
            Serial.printf("%s %04X%04X%04X\n", variant.data(), serial[0], serial[1], serial[2]);
        }
        if (command(0x21B1U))
        {
            state = State::measuring;
            deadline = millis() + 5'000UL;
        }
        else
        {
            state = State::absent;
        }
        return;
    }
    deadline = millis() + 1'000UL;
    std::array<uint16_t, 1U> status{};
    if (read(0xE4B8U, status) && (status[0] & 0x07FFU) != 0U)
    {
        std::array<uint16_t, 3U> measurement{};
        if (read(0xEC05U, measurement) && measurement[0] != 0U && measurement[0] != co2)
        {
            co2 = measurement[0];
            Co2Mode::set(co2);
            pending = true;
        }
    }
    if (pending)
    {
        pending = false;
        transmit();
    }
}

bool Scd4xExtension::command(uint16_t code)
{
    TwoWire &wire{getWire()};
    wire.beginTransmission(address);
    wire.write(static_cast<uint8_t>(code >> 8U));
    wire.write(static_cast<uint8_t>(code));
    return wire.endTransmission() == 0U;
}

uint16_t Scd4xExtension::getCo2() const { return co2; }

uint8_t Scd4xExtension::getCrc(uint8_t msb, uint8_t lsb)
{
    uint8_t crc{0xFFU};
    for (const uint8_t byte : {msb, lsb})
    {
        crc ^= byte;
        for (uint8_t bit{0U}; bit < 8U; ++bit)
        {
            crc = (crc & 0x80U) != 0U ? static_cast<uint8_t>((crc << 1U) ^ 0x31U) : static_cast<uint8_t>(crc << 1U);
        }
    }
    return crc;
}

TwoWire &Scd4xExtension::getWire()
{
    if constexpr (sda % 4 == 0)
    {
        return Wire;
    }
    else
    {
        return Wire1;
    }
}

void Scd4xExtension::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    if (state == State::absent)
    {
        return;
    }
    topic.append(name);
    {
        const std::string id{std::string(name).append("_co2")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::device_class].set("carbon_dioxide");
        component[HomeAssistant::Abbreviations::name].set("CO2");
        component[HomeAssistant::Abbreviations::platform].set("sensor");
        component[HomeAssistant::Abbreviations::state_class].set("measurement");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::unit_of_measurement].set("ppm");
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.co2}}");
    }
}

bool Scd4xExtension::read(uint16_t code, std::span<uint16_t> words)
{
    if (!command(code))
    {
        return false;
    }
    delay(1UL);
    TwoWire &wire{getWire()};
    const size_t length{words.size() * 3U};
    if (wire.requestFrom(address, length) != length)
    {
        return false;
    }
    for (uint16_t &word : words)
    {
        const uint8_t msb{static_cast<uint8_t>(wire.read())};
        const uint8_t lsb{static_cast<uint8_t>(wire.read())};
        if (getCrc(msb, lsb) != static_cast<uint8_t>(wire.read()))
        {
            return false;
        }
        word = static_cast<uint16_t>((msb << 8U) | lsb);
    }
    return true;
}

void Scd4xExtension::resume()
{
    if constexpr (enabled)
    {
        if (command(0x3F86U))
        {
            state = State::stopping;
            deadline = millis() + 500UL;
        }
        else
        {
            state = State::absent;
        }
    }
}

void Scd4xExtension::suspend()
{
    if (state != State::absent)
    {
        static_cast<void>(command(0x3F86U));
        state = State::absent;
    }
}

void Scd4xExtension::transmit()
{
    JsonDocument doc{};
    doc["co2"].set(co2);
    doc["variant"].set(variant);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
