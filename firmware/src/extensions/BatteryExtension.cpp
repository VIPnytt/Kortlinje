#include "extensions/BatteryExtension.h"

#include "config/secrets.h"
#include "pio/battery.pio.h"
#include "services/DisplayService.h"

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pll.h>
#include <hardware/xosc.h>
#include <pico/cyw43_arch.h>

void BatteryExtension::configure()
{
#ifdef RP2350_ERRATA_E9
    pio_add_program_at_offset(pio2, &battery_e9_program, PIO_INSTRUCTION_COUNT - battery_e9_program.length);
    pio_sm_claim(pio2, NUM_PIO_STATE_MACHINES - 1U);
#endif // RP2350_ERRATA_E9
}

void BatteryExtension::begin()
{
#ifdef RP2350_ERRATA_E9
    pio_sm_config config{battery_e9_program_get_default_config(PIO_INSTRUCTION_COUNT - battery_e9_program.length)};
    sm_config_set_set_pins(&config, PIN_WAKE, 1U);
    sm_config_set_in_pins(&config, PIN_WAKE);
    sm_config_set_jmp_pin(&config, PIN_WAKE);
    sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU) / (0b1UL << 20U), 0U);
    pio_gpio_init(pio2, PIN_WAKE);
    gpio_disable_pulls(PIN_WAKE);
    gpio_set_drive_strength(PIN_WAKE, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    pio_sm_set_pins(pio2, NUM_PIO_STATE_MACHINES - 1U, 0U);
    pio_sm_set_consecutive_pindirs(pio2, NUM_PIO_STATE_MACHINES - 1U, PIN_WAKE, 1U, false);
    pio_sm_init(pio2,
                NUM_PIO_STATE_MACHINES - 1U,
                PIO_INSTRUCTION_COUNT - battery_e9_program.length + battery_e9_offset_vbus,
                &config);
    pio_sm_clear_fifos(pio2, NUM_PIO_STATE_MACHINES - 1U);
    pio_interrupt_clear(pio2, 3U);
    pio_set_irq1_source_enabled(pio2, pio_interrupt_source::pis_interrupt3, true);
    irq_set_exclusive_handler(irq_num_rp2350::PIO2_IRQ_1, &onInterrupt);
    irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, true);
    pio_sm_set_enabled(pio2, NUM_PIO_STATE_MACHINES - 1U, true);
#else
    pinMode(PIN_WAKE, PinMode::INPUT);
    attachInterrupt(PIN_WAKE, &onInterrupt, PinStatus::FALLING);
#endif // RP2350_ERRATA_E9
}

void BatteryExtension::handle()
{
    if (pending)
    {
        sleep();
        pending = false;
    }
}

#ifdef RP2350_ERRATA_E9
void __time_critical_func(BatteryExtension::onInterrupt)()
{
    pio_interrupt_clear(pio2, 3U);
    pending = true;
}
#else
void BatteryExtension::onInterrupt() { pending = true; }
#endif // RP2350_ERRATA_E9

void BatteryExtension::sleep()
{
    Display.setPower(false);
    Serial.flush();
    Serial.end();
#ifdef PICO_CYW43_SUPPORTED
    cyw43_arch_deinit();
#endif // PICO_CYW43_SUPPORTED
#ifdef RP2350_ERRATA_E9
    irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, false);
    pio_sm_set_enabled(pio2, NUM_PIO_STATE_MACHINES - 1U, false);
#else
    detachInterrupt(PIN_WAKE);
#endif // RP2350_ERRATA_E9
    gpio_init(PIN_WAKE);
    gpio_disable_pulls(PIN_WAKE);
    gpio_set_input_enabled(PIN_WAKE, true);
    gpio_set_dir(PIN_WAKE, false);
    gpio_set_dormant_irq_enabled(PIN_WAKE, static_cast<uint32_t>(gpio_irq_level::GPIO_IRQ_EDGE_RISE), true);
    clock_configure(clock_num_t::clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_XOSC_CLKSRC,
                    12'000'000UL,
                    12'000'000UL);
    clock_stop(clock_num_t::clk_peri);
    clock_stop(clock_num_t::clk_usb);
    clock_stop(clock_num_t::clk_adc);
    pll_deinit(pll_sys);
    pll_deinit(pll_usb);
    xosc_dormant();
    gpio_set_dormant_irq_enabled(PIN_WAKE, static_cast<uint32_t>(gpio_irq_level::GPIO_IRQ_EDGE_RISE), false);
    set_sys_clock_khz(static_cast<uint32_t>(F_CPU) / 1'000UL, true);
#ifdef PICO_CYW43_SUPPORTED
    cyw43_arch_init();
#endif // PICO_CYW43_SUPPORTED
    Serial.begin();
    Display.setPower(true);
#ifdef RP2350_ERRATA_E9
    pio_gpio_init(pio2, PIN_WAKE);
    pio_interrupt_clear(pio2, 3U);
    pio_sm_restart(pio2, NUM_PIO_STATE_MACHINES - 1U);
    pio_sm_clkdiv_restart(pio2, NUM_PIO_STATE_MACHINES - 1U);
    pio_sm_exec(pio2, NUM_PIO_STATE_MACHINES - 1U, pio_encode_jmp(PIO_INSTRUCTION_COUNT - battery_e9_program.length));
    irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, true);
    pio_sm_set_enabled(pio2, NUM_PIO_STATE_MACHINES - 1U, true);
#else
    attachInterrupt(PIN_WAKE, &onInterrupt, PinStatus::FALLING);
#endif // RP2350_ERRATA_E9
}
