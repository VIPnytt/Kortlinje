#include "services/DeviceService.h"

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/ModesService.h"

#include <WiFi.h>

void DeviceService::begin0()
{
    Serial.begin(115'200U);
    WiFi.setHostname("kortlinje");
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    Extensions.begin();
    Modes.begin();
}

void DeviceService::begin1()
{
    analogReadResolution(ADC_RESOLUTION);
    pinMode(A3, INPUT);
    Display.begin();
}

void DeviceService::handle0()
{
    Extensions.handle();
    Modes.handle();
}

void DeviceService::handle1()
{
    Display.flush();
    if (millis() - lastMillis > 0b1U << 16U)
    {
        lastMillis = millis();
        temperature = analogReadTemp();
        Serial.printf("[Device] Temperature: %.2f °C\n", temperature);
        voltage = analogRead(A3);
        const float _voltage{static_cast<float>(voltage) * 9.9F / 4095.0F};
        Serial.printf("[Device] Voltage: %.2fV\n", _voltage);
    }
}

DeviceService &DeviceService::getInstance()
{
    static DeviceService instance;
    return instance;
}

DeviceService &Device{DeviceService::getInstance()};
