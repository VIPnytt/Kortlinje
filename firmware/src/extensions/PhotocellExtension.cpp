#include "extensions/PhotocellExtension.h"

#include "config/secrets.h"
#include "pio/photocell.pio.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <ArduinoJson.h>
#include <hardware/clocks.h>
#include <hardware/irq.h>

PhotocellExtension::PhotocellExtension() : ExtensionModule(name)
{
#ifdef RP2350_ERRATA_E9
    pio_add_program_at_offset(pio2, &photocell_e9_program, 0U);
#else
    pio_add_program_at_offset(pio2, &photocell_program, 0U);
#endif // RP2350_ERRATA_E9
    pio_sm_claim(pio2, 0U);
}

void PhotocellExtension::begin()
{
#ifdef RP2350_ERRATA_E9
    pio_sm_config config{photocell_e9_program_get_default_config(0u)};
    sm_config_set_set_pins(&config, PIN_CDS, 1U);
#else
    pio_sm_config config{photocell_program_get_default_config(0u)};
#endif // RP2350_ERRATA_E9
    sm_config_set_in_pins(&config, PIN_CDS);
    sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU / 2'000'000L), 0U);
    sm_config_set_in_shift(&config, false, false, 0U);
    pio_gpio_init(pio2, PIN_CDS);
    gpio_disable_pulls(PIN_CDS);
    gpio_set_input_enabled(PIN_CDS, true);
#ifdef RP2350_ERRATA_E9
    gpio_set_drive_strength(PIN_CDS, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    pio_sm_set_consecutive_pindirs(pio2, 0U, PIN_CDS, 1U, false);
#endif // RP2350_ERRATA_E9
    pio_sm_init(pio2, 0U, 0u, &config);
    pio_set_irq1_source_enabled(pio2, pio_interrupt_source::pis_interrupt1, true);
    irq_set_exclusive_handler(irq_num_rp2350::PIO2_IRQ_1, &onInterrupt);
    irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, true);
    if (enabled)
    {
        pio_sm_set_enabled(pio2, 0U, true);
    }
}

void PhotocellExtension::handle()
{
    if (enabled && state && !detected)
    {
        lastMillis = millis();
        if (!detected)
        {
            detected = true;
            pending = true;
        }
    }
    else if (enabled && detected && !state && millis() - lastMillis > 0b1UL << 15U)
    {
        detected = false;
        pending = true;
    }
    else if (pending)
    {
        pending = false;
        if (enabled)
        {
            Display.setPower(detected);
        }
        transmit();
    }
}

bool PhotocellExtension::getDetected() const { return detected; }

bool PhotocellExtension::getEnabled() const { return enabled; }

void __time_critical_func(PhotocellExtension::onInterrupt)()
{
    pio_interrupt_clear(pio2, 1U);
    while (!pio_sm_is_rx_fifo_empty(pio2, 0U))
    {
        state = pio_sm_get(pio2, 0U) != 0U;
        lastMillis = millis();
        pending = true;
    }
}

void PhotocellExtension::setEnabled(bool enable)
{
    if (enable != enabled)
    {
        enabled = enable;
        if (enabled)
        {
            lastMillis = millis();
            state = false;
            detected = false;
            pio_sm_clear_fifos(pio2, 0U);
            pio_interrupt_clear(pio2, 1U);
            pio_sm_exec(pio2, 0U, pio_encode_jmp(0u));
            pio_sm_set_enabled(pio2, 0U, true);
        }
        else
        {
            pio_sm_set_enabled(pio2, 0U, false);
#ifdef RP2350_ERRATA_E9
            pio_sm_set_consecutive_pindirs(pio2, 0U, PIN_CDS, 1U, false);
#endif // RP2350_ERRATA_E9
            pio_sm_clear_fifos(pio2, 0U);
            pio_interrupt_clear(pio2, 1U);
        }
        pending = true;
    }
}

void PhotocellExtension::transmit()
{
    JsonDocument doc{};
    doc["detected"].set(detected);
    doc["enabled"].set(enabled);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
