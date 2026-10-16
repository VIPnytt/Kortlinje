#include "services/DeviceService.h"

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
#include "config/version.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"
#include "services/ModesService.h"
#include "services/WebServerService.h"

#include <WiFi.h>
#include <time.h>

void DeviceService::begin0()
{
    Serial.begin(115'200U);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
#ifdef TIME_ZONE_POSIX
    setenv("TZ", TIME_ZONE_POSIX, 1);
    tzset();
#endif // TIME_ZONE_POSIX
    WebServer.begin();
    Fonts.begin();
    Extensions.begin();
    Modes.begin();
    operational = true;
}

void DeviceService::begin1()
{
    analogReadResolution(static_cast<int>(ADC_RESOLUTION));
    pinMode(A3, INPUT);
    pinMode(PIN_BUZ, PinMode::OUTPUT_2MA);
    Display.begin();
    transmit();
}

void DeviceService::handle0()
{
    WebServer.handle();
    Extensions.handle();
    Display.handle();
    if (Display.getPower())
    {
        Modes.handle();
    }
}

void DeviceService::handle1()
{
    if (Display.getPower())
    {
        Display.flush();
    }
    if (millis() - lastMillis > UINT16_MAX)
    {
        transmit();
    }
}

JsonObjectConst DeviceService::getTransmits() const { return transmits.as<JsonObjectConst>(); }

void DeviceService::onReceive(JsonObjectConst payload, std::string_view source) {}

void DeviceService::receive(JsonObjectConst payload, std::string_view source, std::string_view destination) const
{
    if (operational)
    {
        if (Device.name == destination)
        {
            Device.onReceive(payload, source);
            return;
        }
        if (Display.name == destination)
        {
            Display.onReceive(payload, source);
            return;
        }
        if (Modes.name == destination)
        {
            Modes.onReceive(payload, source);
            return;
        }
        for (ExtensionModule *extension : Extensions.getAll())
        {
            if (extension->name == destination)
            {
                extension->onReceive(payload, source);
                return;
            }
        }
        ModeModule *mode{Modes.getMode()};
        if (mode != nullptr && mode->name == destination)
        {
            mode->onReceive(payload, source);
            return;
        }
        std::unique_ptr<ModeModule> _mode{Modes.getMode(destination)};
        if (_mode != nullptr)
        {
            _mode->onReceive(payload, source);
        }
    }
}

void DeviceService::transmit()
{
    JsonDocument doc{};
    doc["name"].set(NAME);
    doc["repository"].set("https://github.com/VIPnytt/Kortlinje");
    doc["version"].set(VERSION);
    lastMillis = millis();
    transmit(doc.as<JsonObjectConst>(), name);
}

void DeviceService::transmit(JsonObjectConst payload, std::string_view source, bool retain)
{
    if (retain)
    {
        transmits[source].set(payload);
    }
    if (operational)
    {
        WebServer.onTransmit(payload, source);
    }
}

DeviceService &DeviceService::getInstance()
{
    static DeviceService instance;
    return instance;
}

DeviceService &Device{DeviceService::getInstance()};
