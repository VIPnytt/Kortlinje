#include "extensions/ButtonsTopExtension.h"

#include "config/secrets.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

void ButtonsTopExtension::begin()
{
    pinMode(PIN_TOP, PinMode::INPUT);
    adc_init();
    adc_gpio_init(PIN_TOP);
    adc_select_input(PIN_TOP - __FIRSTANALOGGPIO);
    adc_fifo_setup(true, true, 1U, false, false);
    adc_set_clkdiv(static_cast<float>(0xFFFFU));
    channel = static_cast<uint>(dma_claim_unused_channel(true));
    dma_channel_config config{dma_channel_get_default_config(channel)};
    channel_config_set_transfer_data_size(&config, dma_channel_transfer_size::DMA_SIZE_16);
    channel_config_set_read_increment(&config, false);
    channel_config_set_write_increment(&config, false);
    channel_config_set_dreq(&config, dreq_num_rp2350::DREQ_ADC);
    dma_channel_configure(channel, &config, &raw, &adc_hw->fifo, UINT32_MAX, true);
    adc_run(true);
}

void ButtonsTopExtension::handle()
{
    if (millis() - lastMillis < 0b1U << 4U)
    {
        return;
    }
    lastMillis = millis();
    if (!dma_channel_is_busy(channel))
    {
        dma_channel_set_trans_count(channel, UINT32_MAX, true);
        state = Button::UNKNOWN;
        return;
    }
    const Button _state{state};
    const uint16_t _raw{raw};
    if (_raw >= idle - margin && _raw <= idle)
    {
        state = Button::NONE;
        if (state == _state)
        {
            pressed = false;
        }
    }
    else if (_raw >= high - margin && _raw <= high + margin)
    {
        state = Button::CYCLE;
        if (!pressed && state == _state)
        {
            pressed = true;
            cycle();
        }
    }
    else if (_raw >= medium - margin && _raw <= medium + margin)
    {
        state = Button::BRIGHTNESS;
        if (!pressed && state == _state)
        {
            pressed = true;
            brightness();
        }
    }
    else if (_raw >= low - margin && _raw <= low + margin)
    {
        state = Button::DISMISS;
        if (!pressed && state == _state)
        {
            pressed = true;
            dismiss();
        }
    }
    else if (_raw <= margin)
    {
        state = Button::SNOOZE;
        if (!pressed && state == _state)
        {
            pressed = true;
            snooze();
        }
    }
    else
    {
        state = Button::UNKNOWN;
        if (_raw > idle)
        {
            idle = _raw;
            high = idle * 3U / 4U;
            medium = idle * 2U / 3U;
            low = idle / 2U;
            margin = static_cast<uint8_t>((high - medium) / 3U);
        }
    }
}

void ButtonsTopExtension::brightness() { Display.setPower(!Display.getPower()); }

void ButtonsTopExtension::cycle()
{
    if (Display.getPower())
    {
        Modes.setModeNext();
    }
    else
    {
        Display.setPower(true);
    }
}

void ButtonsTopExtension::dismiss() { tone(PIN_BUZ, 0b1U << 12U, 0b1UL << 7U); }

void ButtonsTopExtension::snooze()
{
    Display.setPixel(DisplayService::Symbol::SNOOZE, !Display.getPixel(DisplayService::Symbol::SNOOZE));
}
