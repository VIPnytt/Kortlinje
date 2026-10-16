#include "extensions/EnvironmentExtension.h"

#include "config/secrets.h"
#include "modes/HumidityMode.h"
#include "modes/TemperatureMode.h"

#include <ranges>

void EnvironmentExtension::begin()
{
    pinMode(PIN_PDM, PinMode::INPUT_PULLDOWN);
    pinMode(PIN_EN, PinMode::OUTPUT_2MA);
    attachInterrupt(PIN_PDM, &onChange, PinStatus::CHANGE);
}

void EnvironmentExtension::handle()
{
    if (pending || millis() - lastMillis > 0b1U << 16U)
    {
        pending = false;
        poll();
    }
}

bool EnvironmentExtension::parse(uint16_t idxSync, std::span<uint8_t, 5U> payload)
{
    if (idxSync + 80U >= edges)
    {
        return false;
    }
    std::ranges::fill(payload, 0U);
    for (uint8_t bit{0U}; bit < 8U * payload.size(); ++bit)
    {
        const uint8_t position{static_cast<uint8_t>(bit / 8U)};
        payload[position] <<= 1U;
        if (edgeBuffer[idxSync + 2U + (bit * 2U)].delta > 1'450U)
        {
            payload[position] |= 1U;
        }
    }
    if (payload[0U] != 0xA5U)
    {
        return false;
    }
    return static_cast<uint8_t>(payload[0U] + payload[1U] + payload[2U] + payload[3U]) == payload[4U];
}

void EnvironmentExtension::poll()
{
    lastMillis = millis();
    noInterrupts();
    edges = 0U;
    capturing = true;
    interrupts();
    digitalWrite(PIN_EN, PinStatus::HIGH);
    while ((millis() - lastMillis < 650U) && (edges < 164U))
    {
        delay(1UL);
    }
    capturing = false;
    digitalWrite(PIN_EN, PinStatus::LOW);
    if (edges < 82U)
    {
        return;
    }
    std::array<uint8_t, 5U> payload{};
    for (uint16_t idx{1U}; idx < edges - 80U; idx++)
    {
        if (edgeBuffer[idx].level == PinStatus::HIGH && edgeBuffer[idx].delta >= 3'200U &&
            edgeBuffer[idx].delta <= 4'500U)
        {
            if (parse(idx, payload))
            {
                const float _temperature{static_cast<float>((payload[2U] << 8U) | payload[1U]) / 10.0F};
                if (_temperature != temperature)
                {
                    temperature = _temperature;
                    TemperatureMode::set(temperature);
                }
                if (humidity != payload[3U])
                {
                    humidity = payload[3U];
                    HumidityMode::set(humidity);
                }
                break;
            }
        }
    }
}

void EnvironmentExtension::queue() { pending = true; }

void __attribute__((section(".time_critical.EnvironmentExtension::onChange()"))) EnvironmentExtension::onChange()
{
    const uint32_t now{micros()};
    if (!capturing)
    {
        return;
    }
    const uint16_t _edges{edges};
    if (_edges < edgeBuffer.size())
    {
        edgeBuffer[_edges] = {_edges == 0U ? 0U : now - lastEdge, digitalRead(PIN_PDM)};
        lastEdge = now;
        edges = _edges + 1U;
    }
}
