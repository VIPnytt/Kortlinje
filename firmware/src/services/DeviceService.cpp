#include "services/DeviceService.h"

#include "config/IkeaKortlinje.h"
#include "config/version.h"
#include "extensions/AlarmExtension.h"
#include "extensions/ButtonExtension.h"
#include "extensions/ProvisioningExtension.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"
#include "services/ModesService.h"
#include "services/WebServerService.h"

#include <LittleFS.h>
#include <WiFi.h>
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>
#include <time.h>

void DeviceService::begin0()
{
    LittleFS.begin();
    extensions.configure();
    WiFi.setHostname(HOSTNAME);
    setenv("TZ", TIME_ZONE_POSIX, 1);
    tzset();
    webServer.begin();
    const bool station{ProvisioningExtension::beginStation()};
    extensions.begin();
    if (station)
    {
        NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
        NTP.waitSet(&onWaitSet);
    }
    rp2040.fifo.push(1UL);
}

void DeviceService::begin1()
{
    LittleFS.begin();
    display.begin();
    fonts.begin();
    static_cast<void>(rp2040.fifo.pop());
    modes.begin();
}

void DeviceService::handle0()
{
    extensions.handle();
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
    display.handle();
}

void DeviceService::handle1()
{
    if (display.getPower())
    {
        modes.handle();
        display.flush();
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

JsonObjectConst DeviceService::getTransmits() { return transmits.as<JsonObjectConst>(); }

void DeviceService::onReceive(JsonObjectConst payload, std::string_view source) {}

void DeviceService::onWaitSet() { AlarmExtension::reschedule(); }

void DeviceService::receive(JsonObjectConst payload, std::string_view source, std::string_view destination)
{
    if (name == destination)
    {
        onReceive(payload, source);
        return;
    }
    if (DisplayService::name == destination)
    {
        DisplayService::onReceive(payload, source);
        return;
    }
    if (ModesService::name == destination)
    {
        ModesService::onReceive(payload, source);
        return;
    }
    for (ExtensionModule *extension : ExtensionsService::getAll())
    {
        if (extension->name == destination)
        {
            extension->onReceive(payload, source);
            return;
        }
    }
    ModeModule *mode{ModesService::getMode()};
    if (mode != nullptr && mode->name == destination)
    {
        mode->onReceive(payload, source);
        return;
    }
    std::unique_ptr<ModeModule> _mode{ModesService::getMode(destination)};
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
    for (ExtensionModule *extension : ExtensionsService::getAll())
    {
        extension->onTransmit(payload, source);
    }
}
