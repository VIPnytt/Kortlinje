#include "extensions/BatteryExtension.h"

#include "config/secrets.h"
#include "services/DisplayService.h"

#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pll.h>
#include <hardware/timer.h>
#include <hardware/xosc.h>
#include <pico/cyw43_arch.h>

void BatteryExtension::begin()
{
#ifdef RP2350_ERRATA_E9
    gpio_init(PIN_WAKE);
    gpio_disable_pulls(PIN_WAKE);
    gpio_set_drive_strength(PIN_WAKE, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    gpio_put(PIN_WAKE, false);
    gpio_set_input_enabled(PIN_WAKE, true);
    gpio_set_dir(PIN_WAKE, false);
#else
    pinMode(PIN_WAKE, PinMode::INPUT);
    attachInterrupt(PIN_WAKE, &onFalling, PinStatus::FALLING);
#endif // RP2350_ERRATA_E9
}

void BatteryExtension::handle()
{
#ifdef RP2350_ERRATA_E9
    if (millis() - lastMillis > 0b1UL << 6U)
    {
        lastMillis = millis();
        const bool state{gpio_get(PIN_WAKE)};
        gpio_set_dir(PIN_WAKE, true);
        tight_loop_contents();
        gpio_set_dir(PIN_WAKE, false);
        if (debounce == INT8_MIN)
        {
            sleep();
            debounce = 0;
            lastMillis = millis();
        }
        else if (state && debounce != INT8_MAX)
        {
            ++debounce;
        }
        else if (!state && debounce != INT8_MIN)
        {
            --debounce;
        }
    }
#else
    if (pending)
    {
        pending = false;
        sleep();
    }
#endif // RP2350_ERRATA_E9
}

#ifndef RP2350_ERRATA_E9
void BatteryExtension::onFalling() { pending = true; }
#endif // RP2350_ERRATA_E9

void BatteryExtension::sleep()
{
    Display.setPower(false);
    Serial.flush();
    Serial.end();
#ifdef PICO_CYW43_SUPPORTED
    cyw43_arch_deinit();
#endif // PICO_CYW43_SUPPORTED
#ifndef RP2350_ERRATA_E9
    detachInterrupt(PIN_WAKE);
#endif // RP2350_ERRATA_E9
    gpio_set_dormant_irq_enabled(PIN_WAKE, static_cast<uint32_t>(gpio_irq_level::GPIO_IRQ_EDGE_RISE), true);
    clock_configure(clock_num_t::clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_XOSC_CLKSRC,
                    12'000'000U,
                    12'000'000U);
    clock_stop(clock_num_t::clk_peri);
    clock_stop(clock_num_t::clk_usb);
    clock_stop(clock_num_t::clk_adc);
    pll_deinit(pll_sys);
    pll_deinit(pll_usb);
    xosc_dormant();
    gpio_set_dormant_irq_enabled(PIN_WAKE, static_cast<uint32_t>(gpio_irq_level::GPIO_IRQ_EDGE_RISE), false);
    set_sys_clock_khz(F_CPU / 1'000U, true);
#ifdef PICO_CYW43_SUPPORTED
    cyw43_arch_init();
#endif // PICO_CYW43_SUPPORTED
    Serial.begin();
    Display.setPower(true);
#ifndef RP2350_ERRATA_E9
    attachInterrupt(PIN_WAKE, &onFalling, PinStatus::FALLING);
#endif // RP2350_ERRATA_E9
}
