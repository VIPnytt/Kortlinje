#include "extensions/EnvironmentExtension.h"

#include "config/secrets.h"
#include "modes/TickerMode.h"

#include <format>

void EnvironmentExtension::begin()
{
    pinMode(PIN_PDM, PinMode::INPUT_PULLDOWN);
    pinMode(PIN_EN, PinMode::OUTPUT_2MA);
    attachInterrupt(PIN_PDM, &onChange, PinStatus::CHANGE);
}

void EnvironmentExtension::handle()
{
    if (millis() - lastMillis > 0b1U << 16U)
    {
        lastMillis = millis();
        read();
        TickerMode::setMessage(std::string("Kortlinje ")
                                   .append(std::format("{:.1f}", temperature))
                                   .append("°C ")
                                   .append(std::to_string(humidity))
                                   .append("%"));
    }
}

void EnvironmentExtension::read()
{
    noInterrupts();
    edges = 0U;
    capturing = true;
    interrupts();
    digitalWrite(PIN_EN, PinStatus::HIGH);
    const uint32_t startMillis{millis()};
    while ((millis() - startMillis < 650U) && (edges < 164U))
    {
        delay(1UL);
    }
    capturing = false;
    digitalWrite(PIN_EN, PinStatus::LOW);
    if (edges < 82U)
    {
        return;
    }
    uint8_t payload[5U]{};
    for (uint16_t idx{1U}; idx < edges - 80U; idx++)
    {
        if (edgeBuffer[idx].level == PinStatus::HIGH && edgeBuffer[idx].delta >= SYNC_MIN_US &&
            edgeBuffer[idx].delta <= SYNC_MAX_US)
        {
            if (parse(idx, payload))
            {
                humidity = payload[3U];
                temperature = static_cast<int16_t>((payload[2U] << 8U) | payload[1U]) / 10.0F;
                break;
            }
        }
    }
}

bool EnvironmentExtension::parse(uint16_t idxSync, uint8_t payload[5U])
{
    if (idxSync + 80U >= edges)
    {
        return false;
    }
    memset(payload, 0, 5U);
    for (uint8_t bit{0U}; bit < 40U; ++bit)
    {
        const uint8_t position{bit / 8U};
        payload[position] <<= 1U;
        if (edgeBuffer[idxSync + 2U + (bit * 2U)].delta > BIT_THRESHOLD_US)
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

void __not_in_flash_func(EnvironmentExtension::onChange())
{
    const uint32_t now{micros()};
    if (!capturing)
    {
        return;
    }
    const uint16_t _edges{edges};
    if (_edges < MAX_EDGES)
    {
        edgeBuffer[_edges] = {_edges == 0U ? 0U : now - lastEdge, digitalRead(PIN_PDM)};
        lastEdge = now;
        edges = _edges + 1U;
    }
}
