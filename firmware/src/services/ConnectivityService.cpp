#include "services/ConnectivityService.h"

#include "constants/HomeAssistant.h"
#include "constants/Kortlinje.h"
#include "extensions/AlexaExtension.h"
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"

#include <SimpleMDNS.h>
#include <WiFi.h>
#include <array>
#include <cstring>
#include <hardware/gpio.h>
#include <time.h>

/**
 * @brief Configures Wi-Fi, checks the boot button for hotspot mode, and starts NTP synchronization.
 *
 * Initializes Soft-AP mode only if button_rear reads LOW during boot; otherwise starts station mode.
 */
void ConnectivityService::begin()
{
    WiFi.setHostname(Kortlinje::device::hostname.data());
    setenv("TZ", Kortlinje::time::posix.data(), 1);
    tzset();
    gpio_init(Kortlinje::pin::button_rear);
    gpio_disable_pulls(Kortlinje::pin::button_rear);
    gpio_set_dir(Kortlinje::pin::button_rear, gpio_dir::GPIO_IN);
    sleep_us(0b1U << 4U);
    const bool hotspot{!gpio_get(Kortlinje::pin::button_rear)};
    gpio_deinit(Kortlinje::pin::button_rear);
    if (hotspot)
    {
        initHotspot();
    }
    else
    {
        initStation();
    }
}

void ConnectivityService::handle()
{
    if (pendingConnect)
    {
        pendingConnect = false;
        const std::string ssid{std::move(pendingSsid)};
        const std::string key{std::move(pendingKey)};
        pendingSsid.clear();
        pendingKey.clear();
        connect(ssid, key);
    }
    if (pendingScan)
    {
        pendingScan = false;
        if (!scanning)
        {
            scanning = WiFi.scanNetworks(true) != -2;
        }
    }
    if (scanning)
    {
        const int8_t count{WiFi.scanComplete()};
        if (count >= 0)
        {
            scanning = false;
            onScan(count);
            WiFi.scanDelete();
        }
        else if (count == -2)
        {
            scanning = false;
        }
    }
    if (dnsRunning)
    {
        dns.processNextRequest();
    }
    else
    {
        if (WiFi.connected() && WiFi.localIP().isSet())
        {
            if (!routable)
            {
                onRoutable();
                transmit();
            }
        }
        else if (routable)
        {
            onDisconnected();
        }
        if (pending)
        {
            pending = false;
            if (routable)
            {
                transmit();
            }
            else
            {
                multi.run();
            }
        }
    }
}

/**
 * @brief Saves credentials and either reboots (when in AP mode) or connects in station mode.
 *
 * @param ssid Wi-Fi network name.
 * @param key Wi-Fi network password.
 */
void ConnectivityService::connect(std::string_view ssid, std::string_view key)
{
    const std::string _ssid{ssid};
    const std::string _key{key};
    JsonDocument doc{};
    std::string json{};
    if (ConfigHandler::get(name, "Wi-Fi", json))
    {
        deserializeJson(doc, json);
    }
    doc[_ssid].set(_key.empty() ? nullptr : _key.c_str());
    json.clear();
    serializeJson(doc, json);
    if (ConfigHandler::set(name, "Wi-Fi", json) && dnsRunning)
    {
        dns.stop();
        dnsRunning = false;
        static_cast<void>(WiFi.disconnectAP());
        rp2040.reboot();
    }
    multi.clearAPList();
    multi.addAP(_ssid.c_str(), _key.empty() ? nullptr : _key.c_str());
    const bool connected{multi.run() == WL_CONNECTED && WiFi.localIP().isSet()};
    for (const JsonPairConst pair : doc.as<JsonObjectConst>())
    {
        if (pair.key().c_str() != _ssid)
        {
            multi.addAP(pair.key().c_str(), pair.value().as<const char *>());
        }
    }
    if (connected)
    {
        onRoutable();
    }
    transmit();
}

/**
 * @brief Starts the Wi-Fi access point and wildcard DNS responder for hotspot operation.
 */
void ConnectivityService::initHotspot()
{
    WiFi.mode(WiFiMode_t::WIFI_AP);
    const IPAddress apIp{192, 168, 4, 1};
    WiFi.softAPConfig(apIp, apIp, IPAddress{255, 255, 255, 0});
    WiFi.softAP(Kortlinje::device::name.data());
    dns.setErrorReplyCode(DNSReplyCode::NoError);
    dnsRunning = dns.start(53U, "*", apIp);
    transmit();
}

/**
 * @brief Loads saved Wi-Fi credentials and starts station-mode connection attempts.
 */
void ConnectivityService::initStation()
{
    WiFi.mode(WiFiMode_t::WIFI_STA);
    JsonDocument doc{};
    std::string json{};
    if (ConfigHandler::get(name, "Wi-Fi", json))
    {
        deserializeJson(doc, json);
    }
    if (doc[Kortlinje::wifi::ssid].isUnbound())
    {
        doc[Kortlinje::wifi::ssid].set(Kortlinje::wifi::psk);
        serializeJson(doc, json);
        ConfigHandler::set(name, "Wi-Fi", json);
    }
    multi.clearAPList();
    for (const JsonPairConst pair : doc.as<JsonObjectConst>())
    {
        multi.addAP(pair.key().c_str(), pair.value().as<const char *>());
    }
    if (multi.run() == wl_status_t::WL_CONNECTED && WiFi.localIP().isSet())
    {
        onRoutable();
        transmit();
    }
    add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 18U), &onTimer, this, &timer);
}

/**
 * @brief Handles Wi-Fi disconnection events.
 */
void ConnectivityService::onDisconnected()
{
    routable = false;
    if (mdns)
    {
        MDNS.end();
        mdns = false;
    }
}

/**
 * @brief Adds a Home Assistant diagnostic sensor for Wi-Fi signal strength.
 *
 * @param discovery Home Assistant discovery document to update.
 * @param topic Base topic used to read the signal-strength value.
 * @param unique Prefix used to construct the sensor's unique identifier.
 */
void ConnectivityService::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_rssi")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::device_class].set("signal_strength");
        component[HomeAssistant::Abbreviations::entity_category].set("diagnostic");
        component[HomeAssistant::Abbreviations::name].set("Wi-Fi signal");
        component[HomeAssistant::Abbreviations::platform].set("sensor");
        component[HomeAssistant::Abbreviations::state_class].set("measurement");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::unit_of_measurement].set("dBm");
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.rssi}}");
    }
}

/**
 * @brief Handles incoming Wi-Fi credentials and scan requests.
 *
 * @param payload Incoming payload containing optional Wi-Fi credentials or an action.
 * @param source Source identifier for the payload.
 */
void ConnectivityService::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["ssid"].is<std::string_view>())
    {
        pendingSsid = payload["ssid"].as<std::string_view>();
        pendingKey = payload["key"].is<std::string_view>() ? payload["key"].as<std::string_view>() : std::string_view{};
        pendingConnect = !pendingSsid.empty();
    }
    if (payload["action"].is<std::string_view>() && payload["action"].as<std::string_view>() == "scan")
    {
        pendingScan = true;
    }
}

/**
 * @brief Marks the service as routable and starts mDNS and NTP synchronization.
 */
void ConnectivityService::onRoutable()
{
    if (!routable)
    {
        routable = true;
        NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
        if (!mdns && MDNS.begin(Kortlinje::device::hostname.data()))
        {
            mdns = true;
            MDNS.addService("http", "tcp", 80U);
            MDNS.addService("ws", "tcp", 80U);
            MDNS.enableArduino(2'040U, !Kortlinje::ota::password.empty());
            AlexaExtension::onMdns();
        }
    }
}

/**
 * @brief Processes completed Wi-Fi scans and transmits the discovered networks.
 *
 * @param count Number of discovered networks.
 */
void ConnectivityService::onScan(int8_t count)
{
    if (count > 0)
    {
        JsonDocument doc{};
        for (int8_t idx{0}; idx < count; ++idx)
        {
            const auto index{static_cast<uint8_t>(idx)};
            doc["scan"][idx]["encrypted"].set(WiFi.encryptionType(index) != ENC_TYPE_NONE);
            doc["scan"][idx]["rssi"].set(WiFi.RSSI(index));
            doc["scan"][idx]["ssid"].set(WiFi.SSID(index));
        }
        DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
    }
}

/**
 * @brief Hardware repeating timer ISR that flags periodic telemetry or reconnection checks.
 */
bool __time_critical_func(ConnectivityService::onTimer)(repeating_timer *rt)
{
    static_cast<ConnectivityService *>(rt->user_data)->pending = true;
    return true;
}

/**
 * @brief Transmits the current Wi-Fi connectivity status and saved network names.
 */
void ConnectivityService::transmit()
{
    JsonDocument doc{};
    if (!dnsRunning && WiFi.connected())
    {
        doc["rssi"].set(WiFi.RSSI());
        doc["ssid"].set(WiFi.SSID().c_str());
    }
    {
        std::string json{};
        if (ConfigHandler::get(name, "Wi-Fi", json))
        {
            JsonDocument _saved{};
            if (deserializeJson(_saved, json) == DeserializationError::Code::Ok)
            {
                JsonArray saved{doc["saved"].to<JsonArray>()};
                for (const JsonPairConst pair : _saved.as<JsonObjectConst>())
                {
                    saved.add(pair.key());
                }
            }
        }
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
