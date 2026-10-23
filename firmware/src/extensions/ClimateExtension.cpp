#include "extensions/ClimateExtension.h"

#include "config/secrets.h"
#include "modes/HumidityMode.h"
#include "modes/TemperatureMode.h"
#include "pio/climate.pio.h"
#include "services/DeviceService.h"

#include <algorithm>
#include <hardware/clocks.h>
#include <hardware/irq.h>

ClimateExtension::ClimateExtension() : ExtensionModule(name)
{
    pio_add_program_at_offset(pio1, &climate_sampler_program, 0U);
    pio_add_program_at_offset(pio1, &climate_pacer_program, climate_sampler_program.length);
    pio_sm_claim(pio1, 0U);
    pio_sm_claim(pio1, 1U);
}

void ClimateExtension::begin()
{
    pio_sm_config pacer{climate_pacer_program_get_default_config(climate_sampler_program.length)};
    pio_sm_config sampler{climate_sampler_program_get_default_config(0U)};
    sm_config_set_set_pins(&sampler, PIN_EN, 1U);
    sm_config_set_jmp_pin(&pacer, PIN_EN);
    sm_config_set_in_pins(&sampler, PIN_PDM);
    sm_config_set_in_shift(&sampler, false, false, 32U);
    sm_config_set_clkdiv_int_frac8(&pacer, 0U, 0U);
    sm_config_set_clkdiv_int_frac8(&sampler, static_cast<uint32_t>(F_CPU / 10'000L), 0U);
    pio_gpio_init(pio1, PIN_EN);
    pio_gpio_init(pio1, PIN_PDM);
    gpio_pull_down(PIN_PDM);
    gpio_set_input_enabled(PIN_EN, true);
    gpio_set_drive_strength(PIN_EN, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    pio_sm_set_consecutive_pindirs(pio1, 0U, PIN_EN, 1U, true);
    pio_sm_set_consecutive_pindirs(pio1, 0U, PIN_PDM, 1U, false);
    irq_set_exclusive_handler(irq_num_rp2350::PIO1_IRQ_0, &onSampler);
    irq_set_exclusive_handler(irq_num_rp2350::PIO1_IRQ_1, &onPacer);
    irq_set_enabled(irq_num_rp2350::PIO1_IRQ_0, true);
    irq_set_enabled(irq_num_rp2350::PIO1_IRQ_1, true);
    pio_set_irq0_source_enabled(pio1, pio_interrupt_source::pis_interrupt2, true);
    pio_set_irq1_source_enabled(pio1, pio_interrupt_source::pis_interrupt3, true);
    pio_sm_init(pio1, 0U, 0U, &sampler);
    pio_sm_init(pio1, 1U, climate_sampler_program.length + 1U, &pacer);
    pio_sm_set_enabled(pio1, 0U, true);
    pio_sm_set_enabled(pio1, 1U, true);
    pio1->irq_force = 1U << 4U;
}

void ClimateExtension::handle()
{
    if (pending)
    {
        pending = false;
        HumidityMode::set(humidity);
        TemperatureMode::set(temperature);
        transmit();
    }
}

void __time_critical_func(ClimateExtension::onPacer)()
{
    pio_interrupt_clear(pio1, 3U);
    pio_sm_set_enabled(pio1, 0U, false);
    pio_sm_clear_fifos(pio1, 0U);
    pio_sm_restart(pio1, 0U);
    pio_sm_clkdiv_restart(pio1, 0U);
    pio_sm_exec(pio1, 0U, pio_encode_set(pio_src_dest::pio_pins, 0U));
    pio_sm_exec(pio1, 0U, pio_encode_jmp(0U));
    pio_sm_set_enabled(pio1, 0U, true);
}

void __time_critical_func(ClimateExtension::onSampler)()
{
    pio_interrupt_clear(pio1, 2U);
    const uint8_t checksum{static_cast<uint8_t>(~pio_sm_get(pio1, 0U))};
    const uint32_t payload{~pio_sm_get(pio1, 0U)};
    if (payload >> 24U == 0xA5U)
    {
        const uint8_t lsb{static_cast<uint8_t>(payload >> 16U)};
        const uint8_t msb{static_cast<uint8_t>(payload >> 8U)};
        const uint8_t _humidity{static_cast<uint8_t>(payload)};
        if (static_cast<uint8_t>(0xA5U + lsb + msb + _humidity) == checksum)
        {
            humidity = _humidity;
#if TEMPERATURE_FAHRENHEIT
            temperature = (static_cast<int16_t>((msb << 8U) | lsb) * 0.18F) + 32.0F;
#elif TEMPERATURE_KELVIN
            temperature = (static_cast<int16_t>((msb << 8U) | lsb) * 0.1F) + 273.15F;
#else
            temperature = static_cast<int16_t>((msb << 8U) | lsb) * 0.1F;
#endif // TEMPERATURE_FAHRENHEIT
            pending = true;
        }
    }
}

void ClimateExtension::transmit()
{
    JsonDocument doc{};
    doc["humidity"].set(humidity);
    doc["temperature"].set(temperature);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
