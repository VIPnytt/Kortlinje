#include "extensions/BatteryExtension.h"

#include "config/secrets.h"

#include <Arduino.h>
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pll.h>
#include <hardware/xosc.h>

void BatteryExtension::begin()
{
    pinMode(PIN_WAKE, PinMode::INPUT);
    attachInterrupt(PIN_WAKE, &onFalling, PinStatus::FALLING);
}

void BatteryExtension::handle()
{
    if (sleep)
    {
        sleep = false;
        Serial.end();
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
        Serial.begin(115'200U);
    }
}

void BatteryExtension::onFalling() { sleep = true; }
