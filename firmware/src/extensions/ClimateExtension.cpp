#include "extensions/ClimateExtension.h"

#include "config/secrets.h"
#include "modes/HumidityMode.h"
#include "modes/TemperatureMode.h"
#include "pio/climate.pio.h"
#include "services/DeviceService.h"

#include <algorithm>

void ClimateExtension::begin()
{
    offset = pio_add_program(pio, &climate_program);
    stateMachine = pio_claim_unused_sm(pio, true);
    pio_sm_config config{climate_program_get_default_config(offset)};
    sm_config_set_set_pins(&config, PIN_EN, 1U);
    pio_gpio_init(pio, PIN_EN);
    pio_sm_set_consecutive_pindirs(pio, stateMachine, PIN_EN, 1U, true);
    sm_config_set_in_pins(&config, PIN_PDM);
    sm_config_set_jmp_pin(&config, PIN_PDM);
    pio_gpio_init(pio, PIN_PDM);
    pio_sm_set_consecutive_pindirs(pio, stateMachine, PIN_PDM, 1U, false);
    sm_config_set_in_shift(&config, false, true, 32U);
    sm_config_set_clkdiv(&config, static_cast<float>(clock_get_hz(clock_num_rp2350::clk_sys)) / 20'000.0F);
    irq_set_exclusive_handler(irq_num_rp2350::PIO1_IRQ_0, &onInterrupt);
    irq_set_enabled(irq_num_rp2350::PIO1_IRQ_0, true);
    pio_set_irq0_source_enabled(pio, pio_interrupt_source::pis_interrupt0, true);
    pio_sm_init(pio, stateMachine, offset, &config);
    pio_sm_set_enabled(pio, stateMachine, true);
    lastMillis = millis();
    pio_sm_put(pio, stateMachine, 1U);
}

void ClimateExtension::handle()
{
    if (pending)
    {
        pending = false;
        transmit();
    }
    else if (millis() - lastMillis > 0b1U << 16U)
    {
        sample();
    }
}

uint8_t ClimateExtension::getHumidity() const { return humidity; }

float ClimateExtension::getTemperature() const { return temperature; }

void ClimateExtension::sample()
{
    if (millis() - lastMillis > 0b1U << 10U)
    {
        lastMillis = millis();
        pio_sm_clear_fifos(pio, stateMachine);
        pio_sm_put(pio, stateMachine, 1U);
    }
}

void ClimateExtension::transmit()
{
    HumidityMode::set(humidity);
    TemperatureMode::set(temperature);
    JsonDocument doc{};
    doc["humidity"].set(humidity);
    doc["temperature"].set(temperature);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}

void __attribute__((section(".time_critical.ClimateExtension::onInterrupt"))) ClimateExtension::onInterrupt()
{
    pio_interrupt_clear(pio, 0U);
    const uint32_t payload{~pio_sm_get(pio, stateMachine)};
    const uint8_t checksum{static_cast<uint8_t>(~pio_sm_get(pio, stateMachine))};
    if ((payload >> 24U) == 0xA5U)
    {
        const uint8_t lsb{static_cast<uint8_t>(payload >> 16U)};
        const uint8_t msb{static_cast<uint8_t>(payload >> 8U)};
        const uint8_t _humidity{static_cast<uint8_t>(payload)};
        if (static_cast<uint8_t>(0xA5U + lsb + msb + _humidity) == checksum)
        {
            humidity = _humidity;
            temperature = static_cast<float>((msb << 8U) | lsb) / 10.0F;
            pending = true;
        }
    }
}
