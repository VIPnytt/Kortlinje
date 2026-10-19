#include "extensions/ButtonsRearExtension.h"

#include "config/secrets.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

void ButtonsRearExtension::begin()
{
    pinMode(PIN_REAR, PinMode::INPUT);
    adc_init();
    adc_gpio_init(PIN_REAR);
    adc_select_input(PIN_REAR - __FIRSTANALOGGPIO);
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

void ButtonsRearExtension::handle()
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
        state = Button::MINUS;
        if (!pressed && state == _state)
        {
            pressed = true;
            minus();
        }
    }
    else if (_raw >= medium - margin && _raw <= medium + margin)
    {
        state = Button::PLUS;
        if (!pressed && state == _state)
        {
            pressed = true;
            plus();
        }
    }
    else if (_raw >= low - margin && _raw <= low + margin)
    {
        state = Button::ALARM;
        if (!pressed && state == _state)
        {
            pressed = true;
            alarm();
        }
    }
    else if (_raw <= margin)
    {
        state = Button::CONFIG;
        if (!pressed && state == _state)
        {
            pressed = true;
            config();
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

void ButtonsRearExtension::alarm()
{
    const bool alarm1{Display.getPixel(DisplayService::Symbol::ALARM1)};
    const bool alarm2{Display.getPixel(DisplayService::Symbol::ALARM2)};
    if (alarm1 && alarm2)
    {
        Display.setPixel(DisplayService::Symbol::ALARM1, false);
        Display.setPixel(DisplayService::Symbol::ALARM2, false);
    }
    else if (alarm1)
    {
        Display.setPixel(DisplayService::Symbol::ALARM1, false);
        Display.setPixel(DisplayService::Symbol::ALARM2, true);
    }
    else if (alarm2)
    {
        Display.setPixel(DisplayService::Symbol::ALARM1, true);
    }
    else
    {
        Display.setPixel(DisplayService::Symbol::ALARM1, true);
    }
}

void ButtonsRearExtension::config()
{
    const bool _am{Display.getPixel(DisplayService::Symbol::AM)};
    const bool _pm{Display.getPixel(DisplayService::Symbol::PM)};
    if (_am && _pm)
    {
        Display.setPixel(DisplayService::Symbol::AM, false);
        Display.setPixel(DisplayService::Symbol::PM, false);
    }
    else if (_am)
    {
        Display.setPixel(DisplayService::Symbol::AM, false);
        Display.setPixel(DisplayService::Symbol::PM, true);
    }
    else if (_pm)
    {
        Display.setPixel(DisplayService::Symbol::AM, true);
    }
    else
    {
        Display.setPixel(DisplayService::Symbol::AM, true);
    }
}

void ButtonsRearExtension::minus()
{
    const DisplayService::Brightness brightness{Display.getBrightness()};
    if (!Display.getPower())
    {
        Display.setPower(true);
    }
    else if (brightness != DisplayService::Brightness::DUTY16_1)
    {
        Display.setBrightness(static_cast<DisplayService::Brightness>(static_cast<uint8_t>(brightness) - 1U));
    }
}

void ButtonsRearExtension::plus()
{
    const DisplayService::Brightness brightness{Display.getBrightness()};
    if (!Display.getPower())
    {
        Display.setPower(true);
    }
    else if (brightness != DisplayService::Brightness::DUTY16_14)
    {
        Display.setBrightness(static_cast<DisplayService::Brightness>(static_cast<uint8_t>(brightness) + 1U));
    }
}
