#include "extensions/BatteryExtension.h"

#include "config/secrets.h"

void BatteryExtension::begin()
{
    pinMode(PIN_BATTERY, PinMode::INPUT);
    raw = analogRead(PIN_BATTERY);
}

void BatteryExtension::handle()
{
    if (millis() - lastMillis < 0b1U << 20U)
    {
        return;
    }
    lastMillis = millis();
    raw = analogRead(PIN_BATTERY);
    const float voltage{static_cast<float>(raw) * 3.3F / 4095.0F};
    Serial.printf("[Battery] Voltage: %.2f V\n", voltage);
}
