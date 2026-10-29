#include "services/DeviceService.h"

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
#include "config/version.h"
#include "extensions/ButtonExtension.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"
#include "services/ModesService.h"
#include "services/WebServerService.h"

#include <WiFi.h>
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>
#include <time.h>

void DeviceService::begin0()
{
    Extensions.configure();
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_KEY);
    NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
    setenv("TZ", TIME_ZONE_POSIX, 1);
    tzset();
    WebServer.begin();
    Extensions.begin();
    rp2040.fifo.push(1UL);
}

void DeviceService::begin1()
{
    LittleFS.begin();
    Display.begin();
    Fonts.begin();
    (void)rp2040.fifo.pop();
    Modes.begin();
}

void DeviceService::handle0()
{
    Extensions.handle();
    if (!ButtonExtension::isEnabled())
    {
        adc_set_clkdiv(.0F);
        adc_fifo_setup(true, false, 0U, false, false);
        adc_set_temp_sensor_enabled(true);
        adc_select_input(__GPIOCNT - __FIRSTANALOGGPIO);
        const uint16_t sumTemperature{getSamples()};
        adc_set_temp_sensor_enabled(false);
#ifdef PICO_CYW43_SUPPORTED
        cyw43_thread_enter();
#endif // PICO_CYW43_SUPPORTED
        adc_gpio_init(A3);
        adc_select_input(A3 - __FIRSTANALOGGPIO);
        const uint16_t sumVsys{getSamples()};
#ifdef PICO_CYW43_SUPPORTED
        cyw43_thread_exit();
#endif // PICO_CYW43_SUPPORTED
        ButtonExtension::enable();
        const float _temperature{static_cast<float>(sumTemperature) *
                                 (3.3F / static_cast<float>(samples * ((0b1U << ADC_RESOLUTION) - 1U)))};
#if TEMPERATURE_FAHRENHEIT
        temperature = 80.6F - ((_temperature - .706F) * (1.8F / .001721F));
#elif TEMPERATURE_KELVIN
        temperature = 300.15F - ((_temperature - .706F) * (1.0F / .001721F));
#else
        temperature = 27.0F - ((_temperature - .706F) * (1.0F / .001721F));
#endif // TEMPERATURE_FAHRENHEIT
        voltage = static_cast<float>(sumVsys) * (9.9F / static_cast<float>(samples * ((0b1U << ADC_RESOLUTION) - 1U)));
        transmit();
    }
    Display.handle();
}

void DeviceService::handle1()
{
    if (Display.getPower())
    {
        Modes.handle();
        Display.flush();
    }
}

uint16_t DeviceService::getSamples()
{
    adc_fifo_drain();
    sleep_us(0b1U << 4U);
    adc_run(true);
    uint16_t sum{0U};
    for (uint8_t idx{0U}; idx < samples; ++idx)
    {
        sum += adc_fifo_get_blocking();
    }
    adc_run(false);
    return sum;
}

JsonObjectConst DeviceService::getTransmits() const { return transmits.as<JsonObjectConst>(); }

void DeviceService::onReceive(JsonObjectConst payload, std::string_view source) {}

void DeviceService::receive(JsonObjectConst payload, std::string_view source, std::string_view destination) const
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

void DeviceService::transmit()
{
    JsonDocument doc{};
    doc["name"].set(NAME);
    doc["repository"].set("https://github.com/VIPnytt/Kortlinje");
    doc["temperature"].set(temperature);
    doc["version"].set(VERSION);
    doc["voltage"].set(voltage);
    transmit(doc.as<JsonObjectConst>(), name);
}

void DeviceService::transmit(JsonObjectConst payload, std::string_view source, bool retain)
{
    if (retain)
    {
        transmits[source].set(payload);
    }
    for (ExtensionModule *extension : Extensions.getAll())
    {
        extension->onTransmit(payload, source);
    }
}

DeviceService &DeviceService::getInstance()
{
    static DeviceService instance;
    return instance;
}

DeviceService &Device{DeviceService::getInstance()};
