#include "extensions/PhotocellExtension.h"

#include "config/secrets.h"
#include "handlers/ConfigHandler.h"
#include "pio/photocell.pio.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <ArduinoJson.h>
#include <hardware/clocks.h>
#include <hardware/irq.h>

void PhotocellExtension::configure()
{
#ifdef RP2350_ERRATA_E9
    pio_add_program_at_offset(pio0, &photocell_e9_program, PIO_INSTRUCTION_COUNT - photocell_e9_program.length);
#else
    pio_add_program_at_offset(pio0, &photocell_program, PIO_INSTRUCTION_COUNT - photocell_program.length);
#endif // RP2350_ERRATA_E9
    pio_sm_claim(pio0, NUM_PIO_STATE_MACHINES - 1U);
}

void PhotocellExtension::begin()
{
    ConfigHandler::get(name, "enabled", enabled);
#ifdef RP2350_ERRATA_E9
    pio_sm_config config{photocell_e9_program_get_default_config(PIO_INSTRUCTION_COUNT - photocell_e9_program.length)};
    sm_config_set_set_pins(&config, PIN_CDS, 1U);
#else
    pio_sm_config config{photocell_program_get_default_config(PIO_INSTRUCTION_COUNT - photocell_program.length)};
#endif // RP2350_ERRATA_E9
    sm_config_set_in_pins(&config, PIN_CDS);
    sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU) / (0b1UL << 21U), 0U);
    sm_config_set_in_shift(&config, false, false, 0U);
    pio_gpio_init(pio0, PIN_CDS);
    gpio_disable_pulls(PIN_CDS);
    gpio_set_input_enabled(PIN_CDS, true);
#ifdef RP2350_ERRATA_E9
    gpio_set_drive_strength(PIN_CDS, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    pio_sm_set_consecutive_pindirs(pio0, NUM_PIO_STATE_MACHINES - 1U, PIN_CDS, 1U, false);
    pio_sm_init(pio0, NUM_PIO_STATE_MACHINES - 1U, PIO_INSTRUCTION_COUNT - photocell_e9_program.length, &config);
#else
    pio_sm_init(pio0, NUM_PIO_STATE_MACHINES - 1U, PIO_INSTRUCTION_COUNT - photocell_program.length, &config);
#endif // RP2350_ERRATA_E9
    pio_sm_clear_fifos(pio0, NUM_PIO_STATE_MACHINES - 1U);
    pio_interrupt_clear(pio0, 1U);
    pio_set_irq1_source_enabled(pio0, pio_interrupt_source::pis_interrupt1, true);
    irq_set_exclusive_handler(irq_num_rp2350::PIO0_IRQ_1, &onPio);
    irq_set_enabled(irq_num_rp2350::PIO0_IRQ_1, true);
    if (enabled)
    {
        pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, true);
    }
}

void PhotocellExtension::handle()
{
    if (pending)
    {
        pending = false;
        if (enabled)
        {
            Display.setPower(detected);
        }
        transmit();
    }
}

void PhotocellExtension::cancelAlarm()
{
    const alarm_id_t _alarm{alarm};
    if (_alarm != 0)
    {
        alarm = 0;
        cancel_alarm(_alarm);
    }
}

bool PhotocellExtension::getDetected() const { return detected; }

bool PhotocellExtension::getEnabled() const { return enabled; }

int64_t __time_critical_func(PhotocellExtension::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<void>(user_data);
    alarm = 0L;
    detected = false;
    pending = true;
    return 0LL;
}

void __time_critical_func(PhotocellExtension::onPio)()
{
    pio_interrupt_clear(pio0, 1U);
    while (!pio_sm_is_rx_fifo_empty(pio0, NUM_PIO_STATE_MACHINES - 1U))
    {
        const bool light{pio_sm_get(pio0, NUM_PIO_STATE_MACHINES - 1U) != 0U};
        if (light)
        {
            cancelAlarm();
            if (!detected)
            {
                detected = true;
                pending = true;
            }
        }
        else if (detected && alarm == 0)
        {
            alarm = add_alarm_in_ms(0b1UL << 15U, &onAlarm, nullptr, true);
        }
    }
}

void PhotocellExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["enabled"].is<bool>())
    {
        setEnabled(payload["enabled"].as<bool>());
    }
}

void PhotocellExtension::setEnabled(bool enable)
{
    if (enable == enabled)
    {
        return;
    }
    enabled = enable;
    cancelAlarm();
    detected = false;
    pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, false);
    pio_sm_clear_fifos(pio0, NUM_PIO_STATE_MACHINES - 1U);
    pio_interrupt_clear(pio0, 1U);
    if (enabled)
    {
#ifdef RP2350_ERRATA_E9
        pio_sm_exec(
            pio0, NUM_PIO_STATE_MACHINES - 1U, pio_encode_jmp(PIO_INSTRUCTION_COUNT - photocell_e9_program.length));
#else
        pio_sm_exec(
            pio0, NUM_PIO_STATE_MACHINES - 1U, pio_encode_jmp(PIO_INSTRUCTION_COUNT - photocell_program.length));
#endif // RP2350_ERRATA_E9
        pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, true);
    }
#ifdef RP2350_ERRATA_E9
    else
    {
        pio_sm_set_consecutive_pindirs(pio0, NUM_PIO_STATE_MACHINES - 1U, PIN_CDS, 1U, false);
    }
#endif // RP2350_ERRATA_E9
    ConfigHandler::set(name, "enabled", enabled);
    pending = true;
}

void PhotocellExtension::transmit()
{
    JsonDocument doc{};
    doc["detected"].set(detected);
    doc["enabled"].set(enabled);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
