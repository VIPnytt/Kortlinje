#include "extensions/BatteryExtension.h"

#include "constants/board.h"
#include "constants/secrets.h"
#include "extensions/AlarmExtension.h"
#include "extensions/Scd4xExtension.h"
#include "pio/battery.pio.h"
#include "services/DisplayService.h"

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/pio.h>
#include <hardware/pll.h>
#include <hardware/watchdog.h>
#include <hardware/xosc.h>
#include <pico/cyw43_arch.h>

void BatteryExtension::configure()
{
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        pio_add_program_at_offset(pio2, &battery_e9_program, PIO_INSTRUCTION_COUNT - battery_e9_program.length);
        pio_sm_claim(pio2, NUM_PIO_STATE_MACHINES - 1U);
    }
}

void BatteryExtension::begin()
{
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        pio_sm_config config{battery_e9_program_get_default_config(PIO_INSTRUCTION_COUNT - battery_e9_program.length)};
        sm_config_set_set_pins(&config, Kortlinje::pin::power_sense, 1U);
        sm_config_set_in_pins(&config, Kortlinje::pin::power_sense);
        sm_config_set_jmp_pin(&config, Kortlinje::pin::power_sense);
        sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU) / (0b1UL << 20U), 0U);
        pio_gpio_init(pio2, Kortlinje::pin::power_sense);
        gpio_disable_pulls(Kortlinje::pin::power_sense);
        gpio_set_drive_strength(Kortlinje::pin::power_sense, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
        pio_sm_set_pins(pio2, NUM_PIO_STATE_MACHINES - 1U, 0U);
        pio_sm_set_consecutive_pindirs(pio2, NUM_PIO_STATE_MACHINES - 1U, Kortlinje::pin::power_sense, 1U, false);
        pio_sm_init(pio2,
                    NUM_PIO_STATE_MACHINES - 1U,
                    PIO_INSTRUCTION_COUNT - battery_e9_program.length + battery_e9_offset_vbus,
                    &config);
        pio_sm_clear_fifos(pio2, NUM_PIO_STATE_MACHINES - 1U);
        pio_interrupt_clear(pio2, 3U);
        pio_set_irq1_source_enabled(pio2, pio_interrupt_source::pis_interrupt3, true);
        irq_set_exclusive_handler(irq_num_rp2350::PIO2_IRQ_1, &onIrq);
        irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, true);
        pio_sm_set_enabled(pio2, NUM_PIO_STATE_MACHINES - 1U, true);
    }
    else
    {
        pinMode(static_cast<pin_size_t>(Kortlinje::pin::power_sense), PinMode::INPUT);
        attachInterrupt(static_cast<pin_size_t>(Kortlinje::pin::power_sense), &onInterrupt, PinStatus::FALLING);
    }
}

void BatteryExtension::handle()
{
    if (pending)
    {
        sleep();
        pending = false;
    }
}

void __time_critical_func(BatteryExtension::onIrq)()
{
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        pio_interrupt_clear(pio2, 3U);
        pending = true;
    }
}

void BatteryExtension::onInterrupt()
{
    if constexpr (Kortlinje::board::type != "RP2350" || Kortlinje::board::revision > 0xA2U)
    {
        pending = true;
    }
}

void BatteryExtension::sleep()
{
    DisplayService::setPower(false);
    Scd4xExtension::suspend();
    Serial.flush();
    Serial.end();
#ifdef PICO_CYW43_SUPPORTED
    cyw43_arch_deinit();
#endif // PICO_CYW43_SUPPORTED
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, false);
        pio_sm_set_enabled(pio2, NUM_PIO_STATE_MACHINES - 1U, false);
    }
    else
    {
        detachInterrupt(static_cast<pin_size_t>(Kortlinje::pin::power_sense));
    }
    gpio_init(Kortlinje::pin::power_sense);
    gpio_disable_pulls(Kortlinje::pin::power_sense);
    gpio_set_input_enabled(Kortlinje::pin::power_sense, true);
    gpio_set_dir(Kortlinje::pin::power_sense, false);
    gpio_set_dormant_irq_enabled(
        Kortlinje::pin::power_sense, static_cast<uint32_t>(gpio_irq_level::GPIO_IRQ_EDGE_RISE), true);
    watchdog_disable();
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
    gpio_set_dormant_irq_enabled(
        Kortlinje::pin::power_sense, static_cast<uint32_t>(gpio_irq_level::GPIO_IRQ_EDGE_RISE), false);
    set_sys_clock_khz(static_cast<uint32_t>(F_CPU) / 1'000UL, true);
    watchdog_enable(16777U, true);
#ifdef PICO_CYW43_SUPPORTED
    cyw43_arch_init();
#endif // PICO_CYW43_SUPPORTED
    Serial.begin();
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        pio_gpio_init(pio2, Kortlinje::pin::power_sense);
        pio_interrupt_clear(pio2, 3U);
        pio_sm_restart(pio2, NUM_PIO_STATE_MACHINES - 1U);
        pio_sm_clkdiv_restart(pio2, NUM_PIO_STATE_MACHINES - 1U);
        pio_sm_exec(
            pio2, NUM_PIO_STATE_MACHINES - 1U, pio_encode_jmp(PIO_INSTRUCTION_COUNT - battery_e9_program.length));
        irq_set_enabled(irq_num_rp2350::PIO2_IRQ_1, true);
        pio_sm_set_enabled(pio2, NUM_PIO_STATE_MACHINES - 1U, true);
    }
    else
    {
        attachInterrupt(static_cast<pin_size_t>(Kortlinje::pin::power_sense), &onInterrupt, PinStatus::FALLING);
    }
    DisplayService::setPower(true);
    Scd4xExtension::resume();
    AlarmExtension::reschedule();
}
