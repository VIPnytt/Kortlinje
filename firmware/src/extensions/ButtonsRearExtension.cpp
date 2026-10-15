#include "extensions/ButtonsRearExtension.h"

#include "config/secrets.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

void ButtonsRearExtension::begin() { pinMode(PIN_REAR, PinMode::INPUT); }

void ButtonsRearExtension::handle()
{
    if (millis() - lastMillis < 0b1U << 4U)
    {
        return;
    }
    lastMillis = millis();
    const Button _state{state};
    const int raw{analogRead(PIN_REAR)};
    if (raw >= idle - margin && raw <= idle)
    {
        state = Button::NONE;
        if (state == _state)
        {
            pressed = false;
        }
    }
    else if (raw >= high - margin && raw <= high + margin)
    {
        state = Button::MINUS;
        if (!pressed && state == _state)
        {
            pressed = true;
            minus();
        }
    }
    else if (raw >= medium - margin && raw <= medium + margin)
    {
        state = Button::PLUS;
        if (!pressed && state == _state)
        {
            pressed = true;
            plus();
        }
    }
    else if (raw >= low - margin && raw <= low + margin)
    {
        state = Button::ALARM;
        if (!pressed && state == _state)
        {
            pressed = true;
            alarm();
        }
    }
    else if (raw <= margin)
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
        if (raw > idle)
        {
            idle = raw;
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
