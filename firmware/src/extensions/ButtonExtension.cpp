#include "extensions/ButtonExtension.h"

#include "config/secrets.h"
#include "services/ModesService.h"

void ButtonExtension::begin()
{
    pinMode(PIN_BUTTONS_BACK, PinMode::INPUT);
    pinMode(PIN_BUTTONS_TOP, PinMode::INPUT);
}

void ButtonExtension::handle()
{
    toggle ? top() : back();
    toggle = !toggle;
}

void ButtonExtension::back()
{
    const ButtonLevel _stateBack{stateBack};
    stateBack = parse(static_cast<uint16_t>(analogRead(PIN_BUTTONS_BACK)));
    if (pressBack && stateBack == _stateBack && stateBack == ButtonLevel::LEVEL4)
    {
        pressBack = false;
    }
    else if (!pressBack && stateBack == _stateBack && stateBack == ButtonLevel::LEVEL3)
    {
        pressBack = true;
        backDown();
    }
    else if (!pressBack && stateBack == _stateBack && stateBack == ButtonLevel::LEVEL2)
    {
        pressBack = true;
        backUp();
    }
    else if (!pressBack && stateBack == _stateBack && stateBack == ButtonLevel::LEVEL1)
    {
        pressBack = true;
        backAlarm();
    }
    else if (!pressBack && stateBack == _stateBack && stateBack == ButtonLevel::LEVEL0)
    {
        pressBack = true;
        backSet();
    }
}

void ButtonExtension::top()
{
    const ButtonLevel _stateTop{stateTop};
    stateTop = parse(static_cast<uint16_t>(analogRead(PIN_BUTTONS_TOP)));
    if (pressTop && stateTop == _stateTop && stateTop == ButtonLevel::LEVEL4)
    {
        pressTop = false;
    }
    else if (!pressTop && stateTop == _stateTop && stateTop == ButtonLevel::LEVEL3)
    {
        pressTop = true;
        topCycle();
    }
    else if (!pressTop && stateTop == _stateTop && stateTop == ButtonLevel::LEVEL2)
    {
        pressTop = true;
        topBrightness();
    }
    else if (!pressTop && stateTop == _stateTop && stateTop == ButtonLevel::LEVEL1)
    {
        pressTop = true;
        topAlarm();
    }
    else if (!pressTop && stateTop == _stateTop && stateTop == ButtonLevel::LEVEL0)
    {
        pressTop = true;
        topSnooze();
    }
}

ButtonExtension::ButtonLevel ButtonExtension::parse(uint16_t raw)
{
    if (raw >= level4 - margin && raw <= level4 + margin)
    {
        return ButtonLevel::LEVEL4;
    }
    if (raw >= level3 - margin && raw <= level3 + margin)
    {
        return ButtonLevel::LEVEL3;
    }
    if (raw >= level2 - margin && raw <= level2 + margin)
    {
        return ButtonLevel::LEVEL2;
    }
    if (raw >= level1 - margin && raw <= level1 + margin)
    {
        return ButtonLevel::LEVEL1;
    }
    if (raw <= margin)
    {
        return ButtonLevel::LEVEL0;
    }
    // Serial.printf("[Button] raw: %4u\n", raw);
    return ButtonLevel::INVALID;
}

void ButtonExtension::backAlarm() { Serial.println("ButtonExtension::backAlarm()"); }

void ButtonExtension::backDown()
{
    Serial.println("ButtonExtension::backDown()");
    Modes.setModePrevious();
}

void ButtonExtension::backSet() { Serial.println("ButtonExtension::backSet()"); }

void ButtonExtension::backUp()
{
    Serial.println("ButtonExtension::backUp()");
    Modes.setModeNext();
}

void ButtonExtension::topAlarm() { Serial.println("ButtonExtension::topAlarm()"); }

void ButtonExtension::topBrightness() { Serial.println("ButtonExtension::topBrightness()"); }

void ButtonExtension::topCycle() { Serial.println("ButtonExtension::topCycle()"); }

void ButtonExtension::topSnooze() { Serial.println("ButtonExtension::topSnooze()"); }
