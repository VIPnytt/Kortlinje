#include "extensions/ButtonExtension.h"

#include "config/secrets.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

#include <hardware/adc.h>
#include <hardware/irq.h>

ButtonExtension::ButtonExtension() : ExtensionModule(name) { dma_channel_claim(channel); }

void ButtonExtension::begin()
{
    adc_init();
    adc_gpio_init(PIN_REAR);
    adc_gpio_init(PIN_TOP);
    adc_select_input(PIN_REAR - __FIRSTANALOGGPIO);
    adc_fifo_setup(true, true, 1U, false, false);
    adc_set_clkdiv(divider);
    dma_channel_config_t config{dma_channel_get_default_config(channel)};
    channel_config_set_read_increment(&config, false);
    channel_config_set_write_increment(&config, false);
    channel_config_set_dreq(&config, dreq_num_rp2350::DREQ_ADC);
    channel_config_set_transfer_data_size(&config, dma_channel_transfer_size::DMA_SIZE_16);
    irq_set_exclusive_handler(irq_num_rp2350::DMA_IRQ_1, &onInterrupt);
    irq_set_enabled(irq_num_rp2350::DMA_IRQ_1, true);
    dma_channel_set_irq1_enabled(channel, true);
    dma_channel_configure(channel, &config, &raw, &adc_hw->fifo, count, true);
    adc_run(true);
}

void ButtonExtension::handle()
{
    if (rear.ready)
    {
        rear.ready = false;
        process(rear.state);
    }
    else if (top.ready)
    {
        top.ready = false;
        process(top.state);
    }
}

void ButtonExtension::enable()
{
    enabled = true;
    adc_select_input(active->pin - __FIRSTANALOGGPIO);
    adc_fifo_setup(true, true, 1U, false, false);
    adc_fifo_drain();
    adc_set_clkdiv(divider);
    dma_channel_set_trans_count(channel, count, true);
    adc_run(true);
}

bool ButtonExtension::isEnabled() { return enabled; }

void ButtonExtension::process(Button button)
{
    switch (button)
    {
    case Button::ALARM:
        alarm();
        break;
    case Button::BRIGHTNESS:
        brightness();
        break;
    case Button::SET:
        config();
        break;
    case Button::CYCLE:
        cycle();
        break;
    case Button::DISMISS:
        dismiss();
        break;
    case Button::DOWN:
        minus();
        break;
    case Button::UP:
        plus();
        break;
    case Button::SNOOZE:
        snooze();
        break;
    default:
        break;
    }
}

void __time_critical_func(ButtonExtension::onInterrupt)()
{
    dma_channel_acknowledge_irq1(channel);
    Ladder &ladder{*active};
    const Button _state{ladder.state};
    const uint16_t _raw{raw};
    if (_raw >= ladder.idle - ladder.margin && _raw <= ladder.idle)
    {
        ladder.state = Button::NONE;
        if (ladder.state == _state)
        {
            ladder.pressed = false;
        }
    }
    else if (_raw >= ladder.high - ladder.margin && _raw <= ladder.high + ladder.margin)
    {
        ladder.state = ladder._high;
        if (!ladder.pressed && ladder.state == _state)
        {
            ladder.pressed = true;
            ladder.ready = true;
        }
    }
    else if (_raw >= ladder.medium - ladder.margin && _raw <= ladder.medium + ladder.margin)
    {
        ladder.state = ladder._medium;
        if (!ladder.pressed && ladder.state == _state)
        {
            ladder.pressed = true;
            ladder.ready = true;
        }
    }
    else if (_raw >= ladder.low - ladder.margin && _raw <= ladder.low + ladder.margin)
    {
        ladder.state = ladder._low;
        if (!ladder.pressed && ladder.state == _state)
        {
            ladder.pressed = true;
            ladder.ready = true;
        }
    }
    else if (_raw <= ladder.margin)
    {
        ladder.state = ladder._zero;
        if (!ladder.pressed && ladder.state == _state)
        {
            ladder.pressed = true;
            ladder.ready = true;
        }
    }
    else
    {
        ladder.state = Button::UNKNOWN;
        if (_raw > ladder.idle)
        {
            ladder.idle = _raw;
            ladder.high = ladder.idle * 3U / 4U;
            ladder.medium = ladder.idle * 2U / 3U;
            ladder.low = ladder.idle / 2U;
            ladder.margin = static_cast<uint8_t>((ladder.high - ladder.medium) / 3U);
        }
    }
    active = (active == &rear) ? &top : &rear;
    if (!rear.pressed && !top.pressed && rear.state == Button::NONE && top.state == Button::NONE &&
        ++bursts == 0b1U << 13U)
    {
        adc_run(false);
        enabled = false;
    }
    else
    {
        adc_select_input(active->pin - __FIRSTANALOGGPIO);
        dma_channel_set_trans_count(channel, count, true);
    }
}

void ButtonExtension::alarm()
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

void ButtonExtension::brightness() { Display.setPower(!Display.getPower()); }

void ButtonExtension::config()
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

void ButtonExtension::cycle()
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

void ButtonExtension::dismiss() { tone(PIN_BUZ, 0b1U << 12U, 0b1UL << 7U); }

void ButtonExtension::minus()
{
    const DisplayService::Brightness _brightness{Display.getBrightness()};
    if (!Display.getPower())
    {
        Display.setPower(true);
    }
    else if (_brightness != DisplayService::Brightness::DUTY16_1)
    {
        Display.setBrightness(static_cast<DisplayService::Brightness>(static_cast<uint8_t>(_brightness) - 1U));
    }
}

void ButtonExtension::plus()
{
    const DisplayService::Brightness _brightness{Display.getBrightness()};
    if (!Display.getPower())
    {
        Display.setPower(true);
    }
    else if (_brightness != DisplayService::Brightness::DUTY16_14)
    {
        Display.setBrightness(static_cast<DisplayService::Brightness>(static_cast<uint8_t>(_brightness) + 1U));
    }
}

void ButtonExtension::snooze()
{
    Display.setPixel(DisplayService::Symbol::SNOOZE, !Display.getPixel(DisplayService::Symbol::SNOOZE));
}
