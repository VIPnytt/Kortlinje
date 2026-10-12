#include "extensions/ButtonExtension.h"

#include "config/secrets.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

void ButtonExtension::begin()
{
    pinMode(PIN_REAR, PinMode::INPUT);
    pinMode(PIN_TOP, PinMode::INPUT);
    pinMode(PIN_BUZ, PinMode::OUTPUT_2MA);
    idle = static_cast<uint16_t>((analogRead(PIN_REAR) + analogRead(PIN_TOP)) / 2);
    high = idle * 3U / 4U;
    medium = idle * 2U / 3U;
    low = idle / 2U;
    margin = static_cast<uint8_t>((high - medium) / 2U);
}

void ButtonExtension::handle()
{
    if (millis() - lastMillis > 0b1U << 3U)
    {
        lastMillis = millis();
        toggle ? readTop() : readRear();
        toggle = !toggle;
    }
}

void ButtonExtension::readRear()
{
    const ButtonLevel _stateRear{stateRear};
    stateRear = parse(static_cast<uint16_t>(analogRead(PIN_REAR)));
    if (pressRear && stateRear == _stateRear && stateRear == ButtonLevel::IDLE)
    {
        pressRear = false;
    }
    else if (!pressRear && stateRear == _stateRear && stateRear == ButtonLevel::LEVEL3)
    {
        pressRear = true;
        rearDown();
    }
    else if (!pressRear && stateRear == _stateRear && stateRear == ButtonLevel::LEVEL2)
    {
        pressRear = true;
        rearUp();
    }
    else if (!pressRear && stateRear == _stateRear && stateRear == ButtonLevel::LEVEL1)
    {
        pressRear = true;
        rearAlarm();
    }
    else if (!pressRear && stateRear == _stateRear && stateRear == ButtonLevel::LEVEL0)
    {
        pressRear = true;
        rearSet();
    }
}

void ButtonExtension::readTop()
{
    const ButtonLevel _stateTop{stateTop};
    stateTop = parse(static_cast<uint16_t>(analogRead(PIN_TOP)));
    if (pressTop && stateTop == _stateTop && stateTop == ButtonLevel::IDLE)
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
    if (raw >= idle - margin && raw <= idle + margin)
    {
        return ButtonLevel::IDLE;
    }
    if (raw >= high - margin && raw <= high + margin)
    {
        return ButtonLevel::LEVEL3;
    }
    if (raw >= medium - margin && raw <= medium + margin)
    {
        return ButtonLevel::LEVEL2;
    }
    if (raw >= low - margin && raw <= low + margin)
    {
        return ButtonLevel::LEVEL1;
    }
    if (raw <= margin)
    {
        return ButtonLevel::LEVEL0;
    }
    return ButtonLevel::INVALID;
}

void ButtonExtension::rearAlarm()
{
    Serial.println("ButtonExtension::rearAlarm()");
    tone(PIN_BUZ, 0b1U << 12U, 0b1UL << 8U);
}

void ButtonExtension::rearDown()
{
    Serial.println("ButtonExtension::rearDown()");
    Modes.setModePrevious();
}

void ButtonExtension::rearSet() { Serial.println("ButtonExtension::rearSet()"); }

void ButtonExtension::rearUp()
{
    Serial.println("ButtonExtension::rearUp()");
    Modes.setModeNext();
}

void ButtonExtension::topAlarm()
{
    Serial.println("ButtonExtension::topAlarm()");
    tone(PIN_BUZ, 0b1U << 11U, 0b1UL << 7U);
}

void ButtonExtension::topBrightness()
{
    Serial.println("ButtonExtension::topBrightness()");
    switch (Display.getBrightness())
    {
    case DisplayService::Brightness::DUTY16_1:
        Display.setBrightness(DisplayService::Brightness::DUTY16_14);
        Display.setPower(false);
        break;
    case DisplayService::Brightness::DUTY16_2:
        Display.setBrightness(DisplayService::Brightness::DUTY16_1);
        break;
    case DisplayService::Brightness::DUTY16_4:
        Display.setBrightness(DisplayService::Brightness::DUTY16_2);
        break;
    case DisplayService::Brightness::DUTY16_10:
        Display.setBrightness(DisplayService::Brightness::DUTY16_4);
        break;
    case DisplayService::Brightness::DUTY16_11:
        Display.setBrightness(DisplayService::Brightness::DUTY16_10);
        break;
    case DisplayService::Brightness::DUTY16_12:
        Display.setBrightness(DisplayService::Brightness::DUTY16_11);
        break;
    case DisplayService::Brightness::DUTY16_13:
        Display.setBrightness(DisplayService::Brightness::DUTY16_12);
        break;
    case DisplayService::Brightness::DUTY16_14:
        Display.setBrightness(DisplayService::Brightness::DUTY16_13);
        break;
    }
}

void ButtonExtension::topCycle()
{
    Serial.println("ButtonExtension::topCycle()");
    Modes.setModeNext();
}

void ButtonExtension::topSnooze() { Serial.println("ButtonExtension::topSnooze()"); }
