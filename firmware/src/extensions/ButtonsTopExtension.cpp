#include "extensions/ButtonsTopExtension.h"

#include "config/secrets.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

void ButtonsTopExtension::begin() { pinMode(PIN_TOP, PinMode::INPUT); }

void ButtonsTopExtension::handle()
{
    if (millis() - lastMillis < 0b1U << 4U)
    {
        return;
    }
    lastMillis = millis();
    const Button _state{state};
    const int raw{analogRead(PIN_TOP)};
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
        state = Button::CYCLE;
        if (!pressed && state == _state)
        {
            pressed = true;
            cycle();
        }
    }
    else if (raw >= medium - margin && raw <= medium + margin)
    {
        state = Button::BRIGHTNESS;
        if (!pressed && state == _state)
        {
            pressed = true;
            brightness();
        }
    }
    else if (raw >= low - margin && raw <= low + margin)
    {
        state = Button::DISMISS;
        if (!pressed && state == _state)
        {
            pressed = true;
            dismiss();
        }
    }
    else if (raw <= margin)
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
