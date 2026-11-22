#include "services/DeviceService.h"

#include "constants/HomeAssistant.h"
#include "constants/Kortlinje.h"
#include "extensions/ButtonExtension.h"
#include "extensions/MqttExtension.h"
#include "handlers/CertificateHandler.h"
#include "services/ModesService.h"

#include <HTTPClient.h>
#include <LittleFS.h>
#include <SimpleMDNS.h>
#include <WiFi.h>
#include <hardware/adc.h>
#include <hardware/watchdog.h>
#include <memory>
#include <meta>
#include <pico/cyw43_arch.h>
#include <string>

void DeviceService::begin0()
{
    watchdog_enable(16777U, true);
    Serial.printf("Kortlinje v%s\n", Kortlinje::version.data());
    LittleFS.begin();
    extensions.configure();
    connectivity.begin();
    webServer.begin();
    extensions.begin();
    rp2040.fifo.push(1UL);
    release();
}

void DeviceService::begin1()
{
    display.begin();
    fonts.begin();
    static_cast<void>(rp2040.fifo.pop());
    modes.begin();
}

void DeviceService::handle0()
{
    if (alive.exchange(false, std::memory_order_relaxed))
    {
        watchdog_update();
    }
    connectivity.handle();
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
        if constexpr (Kortlinje::device::temperature_unit == "°C")
        {
            temperature = 27.0F - ((_temperature - .706F) * (1.0F / .001721F));
        }
        else if constexpr (Kortlinje::device::temperature_unit == "°F")
        {
            temperature = 80.6F - ((_temperature - .706F) * (1.8F / .001721F));
        }
        else
        {
            temperature = 300.15F - ((_temperature - .706F) * (1.0F / .001721F));
        }
        voltage = static_cast<float>(sumVsys) * (9.9F / static_cast<float>(samples * ((0b1U << ADC_RESOLUTION) - 1U)));
        transmit();
    }
    display.handle();
}

void DeviceService::handle1()
{
    alive.store(true, std::memory_order_relaxed);
    if (display.getPower())
    {
        modes.handle();
        display.flush();
    }
}

uint16_t DeviceService::getSamples()
{
    adc_fifo_drain();
    sleep_us(0b1UL << 4U);
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

/**
 * @brief Adds Home Assistant discovery definitions for device controls and temperature.
 *
 * @param discovery JSON document to receive the discovery components.
 * @param unique Prefix used to create unique component identifiers.
 */
void DeviceService::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
{
    constexpr std::string_view stateTopic{std::define_static_string(std::string{MqttExtension::topicPrefix} + name)};
    {
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_temperature")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::device_class].set("temperature");
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("diagnostic");
        component[HomeAssistant::Abbreviations::name].set("Internal temperature");
        component[HomeAssistant::Abbreviations::platform].set("sensor");
        component[HomeAssistant::Abbreviations::state_class].set("measurement");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::unit_of_measurement].set(Kortlinje::device::temperature_unit);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.temperature}}");
    }
    {
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_update")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::device_class].set("firmware");
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("diagnostic");
        component[HomeAssistant::Abbreviations::latest_version_template].set("{{value_json.latest}}");
        component[HomeAssistant::Abbreviations::latest_version_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::name].set("Firmware");
        component[HomeAssistant::Abbreviations::platform].set("update");
        component[HomeAssistant::Abbreviations::release_url].set(
            "https://github.com/VIPnytt/Kortlinje/releases/latest");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::title].set("Kortlinje");
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.version}}");
    }
    {
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_vsys")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::device_class].set("voltage");
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("diagnostic");
        component[HomeAssistant::Abbreviations::icon].set("mdi:current-dc");
        component[HomeAssistant::Abbreviations::name].set("System voltage");
        component[HomeAssistant::Abbreviations::platform].set("sensor");
        component[HomeAssistant::Abbreviations::state_class].set("measurement");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::unit_of_measurement].set("V");
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.voltage}}");
    }
}

void DeviceService::receive(JsonObjectConst payload, std::string_view source, std::string_view destination)
{
    if (ConnectivityService::name == destination)
    {
        ConnectivityService::onReceive(payload, source);
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
    ExtensionModule *_extension{ExtensionsService::getExtension(destination)};
    if (_extension != nullptr)
    {
        _extension->onReceive(payload, source);
        return;
    }
    ModeModule *mode{ModesService::getMode()};
    if (mode != nullptr && mode->name == destination)
    {
        mode->onReceive(payload, source);
        return;
    }
    const std::unique_ptr<ModeModule> _mode{ModesService::getMode(destination)};
    if (_mode != nullptr)
    {
        _mode->onReceive(payload, source);
    }
}

void DeviceService::release()
{
    if (std::ranges::contains(resetAbnormalities, rp2040.getResetReason()))
    {
        return;
    }
    BearSSL::WiFiClientSecure secure{};
    secure.setTrustAnchors(&CertificateHandler::list());
    HTTPClient http{};
    http.setFollowRedirects(followRedirects_t::HTTPC_DISABLE_FOLLOW_REDIRECTS);
    if (!http.begin(secure, "https://github.com/VIPnytt/Kortlinje/releases/latest"))
    {
        return;
    }
    http.setUserAgent(std::define_static_string(std::string{"Kortlinje/"} + Kortlinje::version +
                                                " (+https://github.com/VIPnytt/Kortlinje)"));
    if (http.sendRequest("HEAD") == t_http_codes::HTTP_CODE_FOUND)
    {
        constexpr std::string_view tagMarker{"/releases/tag/"};
        const auto &location{http.getLocation()};
        std::string_view view{location.c_str(), location.length()};
        if (const size_t pos{view.find(tagMarker)}; pos != std::string_view::npos)
        {
            view.remove_prefix(pos + tagMarker.size());
            if (view.starts_with('v'))
            {
                view.remove_prefix(1U);
            }
            latest = view;
        }
    }
    http.end();
}

void DeviceService::transmit()
{
    JsonDocument doc{};
    if (!latest.empty())
    {
        doc["latest"].set(latest);
    }
    doc["name"].set(Kortlinje::device::name);
    doc["repository"].set("https://github.com/VIPnytt/Kortlinje");
    doc["temperature"].set(temperature);
    doc["version"].set(Kortlinje::version);
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
