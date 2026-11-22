#include "services/ConnectivityService.h"

#include "constants/HomeAssistant.h"
#include "constants/secrets.h"
#include "extensions/AlexaExtension.h"
#include "extensions/MqttExtension.h"
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"

#include <SimpleMDNS.h>
#include <WiFi.h>
#include <array>
#include <cstring>
#include <hardware/gpio.h>
#include <hardware/watchdog.h>
#include <meta>
#include <string>
#include <time.h>
#include <utility>

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
    sleep_us(0b1UL << 4U);
    const bool button{!gpio_get(Kortlinje::pin::button_rear)};
    gpio_deinit(Kortlinje::pin::button_rear);
    button ? initHotspot() : initStation();
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
    else if (pendingScan)
    {
        pendingScan = false;
        if (!scanning)
        {
            scanning = WiFi.scanNetworks(true) != -2;
        }
    }
    else if (scanning)
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
    else if (dnsRunning)
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
        else if (tick)
        {
            tick = false;
            alarmId = 0L;
            if (routable)
            {
                transmit();
                schedule(0b1ULL << 28U);
            }
            else
            {
                watchdog_update();
                multi.run();
                watchdog_update();
                schedule(0b1ULL << 29U);
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
    watchdog_update();
    const bool connected{multi.run(0b1U << 14U) == wl_status_t::WL_CONNECTED && WiFi.localIP().isSet()};
    watchdog_update();
    for (const JsonPairConst pair : doc.as<JsonObjectConst>())
    {
        if (pair.key().c_str() != _ssid)
        {
            multi.addAP(pair.key().c_str(), pair.value().as<const char *>());
        }
    }
    connected ? onRoutable() : onDisconnected();
    transmit();
}

/**
 * @brief Starts the Wi-Fi access point and wildcard DNS responder for hotspot operation.
 */
void ConnectivityService::initHotspot()
{
    WiFi.mode(WiFiMode_t::WIFI_AP);
    const IPAddress apIp{192U, 168U, 4U, 1U};
    WiFi.softAPConfig(apIp, apIp, IPAddress{255U, 255U, 255U, 0U});
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
    watchdog_update();
    const bool connected{multi.run(0b1U << 14U) == wl_status_t::WL_CONNECTED && WiFi.localIP().isSet()};
    watchdog_update();
    if (connected)
    {
        onRoutable();
        transmit();
    }
    else
    {
        schedule(0b1ULL << 25U);
    }
}

int64_t __time_critical_func(ConnectivityService::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<ConnectivityService *>(user_data)->tick = true;
    return 0LL;
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
    schedule(0b1ULL << 26U);
}

/**
 * @brief Adds a Home Assistant diagnostic sensor for Wi-Fi signal strength.
 *
 * @param discovery Home Assistant discovery document to update.
 * @param unique Prefix used to construct the sensor's unique identifier.
 */
void ConnectivityService::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
{
    constexpr std::string_view stateTopic{std::define_static_string(std::string{MqttExtension::topicPrefix} + name)};
    constexpr std::string_view id{std::define_static_string(std::string{name} + "_rssi")};
    JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
    component[HomeAssistant::Abbreviations::device_class].set("signal_strength");
    component[HomeAssistant::Abbreviations::entity_category].set("diagnostic");
    component[HomeAssistant::Abbreviations::name].set("Wi-Fi signal");
    component[HomeAssistant::Abbreviations::platform].set("sensor");
    component[HomeAssistant::Abbreviations::state_class].set("measurement");
    component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
    component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
    component[HomeAssistant::Abbreviations::unit_of_measurement].set("dBm");
    component[HomeAssistant::Abbreviations::value_template].set("{{value_json.rssi}}");
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
        watchdog_update();
        NTP.waitSet(0b1U << 14U);
        watchdog_update();
        if (!mdns && MDNS.begin(Kortlinje::device::hostname.data()))
        {
            mdns = true;
            MDNS.addService("http", "tcp", 80U);
            MDNS.addService("ws", "tcp", 80U);
            MDNS.enableArduino(2'040U, !Kortlinje::ota::password.empty());
            AlexaExtension::onMdns();
        }
    }
    schedule(0b1ULL << 27U);
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
            const auto _index{static_cast<uint8_t>(idx)};
            doc["scan"][_index]["encrypted"].set(WiFi.encryptionType(idx) != ENC_TYPE_NONE);
            doc["scan"][_index]["rssi"].set(WiFi.RSSI(idx));
            doc["scan"][_index]["ssid"].set(WiFi.SSID(idx));
        }
        DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
    }
}

void ConnectivityService::schedule(uint64_t delay)
{
    const alarm_id_t _alarmId{alarmId};
    if (_alarmId > 0L)
    {
        alarmId = 0L;
        cancel_alarm(_alarmId);
    }
    tick = false;
    alarmId = add_alarm_in_us(delay, &onAlarm, this, true);
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
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
