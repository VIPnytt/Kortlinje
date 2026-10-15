#include "services/DeviceService.h"

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/ModesService.h"

#include <WiFi.h>
#include <time.h>

void DeviceService::begin0()
{
    Serial.begin(115'200U);
    WiFi.setHostname("kortlinje");
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
#ifdef TIME_ZONE_POSIX
    setenv("TZ", TIME_ZONE_POSIX, 1);
#endif // TIME_ZONE_POSIX
    tzset();
    Extensions.begin();
    Modes.begin();
}

void DeviceService::begin1()
{
    analogReadResolution(ADC_RESOLUTION);
    pinMode(A3, INPUT);
    pinMode(PIN_BUZ, PinMode::OUTPUT_2MA);
    Display.begin();
}

void DeviceService::handle0()
{
    Extensions.handle();
    Modes.handle();
}

void DeviceService::handle1()
{
    if (Display.getPower())
    {
        Display.flush();
    }
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
