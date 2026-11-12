#include "services/ConnectivityService.h"

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
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
 * Initializes Soft-AP mode only if PIN_REAR reads LOW during boot; otherwise starts station mode.
 */
void ConnectivityService::begin()
{
    WiFi.setHostname(HOSTNAME);
    setenv("TZ", TIME_ZONE_POSIX, 1);
    tzset();
    gpio_init(PIN_REAR);
    gpio_disable_pulls(PIN_REAR);
    gpio_set_dir(PIN_REAR, GPIO_IN);
    sleep_us(0b1U << 4U);
    const bool hotspot{!gpio_get(PIN_REAR)};
    gpio_deinit(PIN_REAR);
    if (hotspot)
    {
        initHotspot();
    }
    else
    {
        initStation();
        NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
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
        handleDns();
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
        dnsUdp.stop();
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
 * @brief Processes incoming UDP DNS queries for the captive portal.
 */
void ConnectivityService::handleDns()
{
    const IPAddress apIp{WiFi.softAPIP()};
    for (uint8_t idx{0U}; idx < 8U; ++idx)
    {
        const int packetSize{dnsUdp.parsePacket()};
        if (packetSize < 12)
        {
            return;
        }
        std::array<uint8_t, 512U> req{};
        const int len{dnsUdp.read(req.data(), req.size())};
        if (len < 12)
        {
            continue;
        }
        int qend{12};
        while (qend < len && req[static_cast<size_t>(qend)] != 0U)
        {
            const int lab{static_cast<int>(req[static_cast<size_t>(qend)])};
            if ((lab & 0xC0) == 0xC0)
            {
                qend += 2;
                break;
            }
            qend += lab + 1;
            if (qend >= len)
            {
                qend = -1;
                break;
            }
        }
        if (qend < 0 || qend >= len)
        {
            continue;
        }
        qend += 1 + 4; // NUL + QTYPE + QCLASS
        if (qend > len || static_cast<size_t>(qend + 16) > req.size())
        {
            continue;
        }
        const uint16_t qtype{
            static_cast<uint16_t>((req[static_cast<size_t>(qend - 4)] << 8U) | req[static_cast<size_t>(qend - 3)])};
        std::array<uint8_t, 512U> resp{};
        std::memcpy(resp.data(), req.data(), static_cast<size_t>(qend));
        resp[2U] = 0x84U; // QR=1, AA=1
        resp[3U] = 0U;    // RCODE=0
        resp[4U] = 0U;
        resp[5U] = 0x1U; // QDCOUNT = 1
        resp[6U] = 0U;
        resp[7U] = 0U; // ANCOUNT
        resp[8U] = 0U;
        resp[9U] = 0U; // NSCOUNT
        resp[10U] = 0U;
        resp[11U] = 0U; // ARCOUNT = 0 (drop EDNS)
        size_t pos{static_cast<size_t>(qend)};
        if (qtype == 0x0001U)
        {
            resp[7U] = 0x1U;
            resp[pos++] = 0xC0U;
            resp[pos++] = 0xCU;
            resp[pos++] = 0U;
            resp[pos++] = 0x1U; // TYPE A
            resp[pos++] = 00U;
            resp[pos++] = 0x1U; // CLASS IN
            resp[pos++] = 0U;
            resp[pos++] = 0U;
            resp[pos++] = 0U;
            resp[pos++] = 0x3CU; // TTL 60s
            resp[pos++] = 0U;
            resp[pos++] = 0x4U;
            resp[pos++] = apIp[0U];
            resp[pos++] = apIp[1U];
            resp[pos++] = apIp[2U];
            resp[pos++] = apIp[3U];
        }
        dnsUdp.beginPacket(dnsUdp.remoteIP(), dnsUdp.remotePort());
        dnsUdp.write(resp.data(), pos);
        dnsUdp.endPacket();
    }
}

/**
 * @brief Starts the Wi-Fi access point and wildcard DNS responder for hotspot operation.
 */
void ConnectivityService::initHotspot()
{
    WiFi.mode(WIFI_AP);
    const IPAddress apIp{192, 168, 4, 1};
    WiFi.softAPConfig(apIp, apIp, IPAddress{255, 255, 255, 0});
    WiFi.softAP(NAME);
    dnsRunning = dnsUdp.begin(53U) != 0U;
    transmit();
}

/**
 * @brief Loads saved Wi-Fi credentials and starts station-mode connection attempts.
 */
void ConnectivityService::initStation()
{
    WiFi.mode(WIFI_STA);
    JsonDocument doc{};
    std::string json{};
    if (ConfigHandler::get(name, "Wi-Fi", json))
    {
        deserializeJson(doc, json);
    }
    if (doc[WIFI_SSID].isUnbound())
    {
        doc[WIFI_SSID].set(WIFI_PSK);
        json.clear();
        serializeJson(doc, json);
        ConfigHandler::set(name, "Wi-Fi", json);
    }
    multi.clearAPList();
    for (const JsonPairConst pair : doc.as<JsonObjectConst>())
    {
        multi.addAP(pair.key().c_str(), pair.value().as<const char *>());
    }
    if (multi.run() == WL_CONNECTED && WiFi.localIP().isSet())
    {
        onRoutable();
        transmit();
    }
    add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 18U), &onTimer, nullptr, &timer);
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
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::device_class].set("signal_strength");
        component[HomeAssistantAbbreviations::entity_category].set("diagnostic");
        component[HomeAssistantAbbreviations::expire_after].set(UINT8_MAX);
        component[HomeAssistantAbbreviations::force_update].set(true);
        component[HomeAssistantAbbreviations::name].set("Wi-Fi signal");
        component[HomeAssistantAbbreviations::platform].set("sensor");
        component[HomeAssistantAbbreviations::state_class].set("measurement");
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::unit_of_measurement].set("dBm");
        component[HomeAssistantAbbreviations::value_template].set("{{value_json.rssi}}");
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
        if (!mdns && MDNS.begin(HOSTNAME))
        {
            mdns = true;
            MDNS.addService("http", "tcp", 80U);
            MDNS.addService("ws", "tcp", 80U);
        }
        NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
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
bool __time_critical_func(ConnectivityService::onTimer)(repeating_timer *timer)
{
    static_cast<void>(timer);
    pending = true;
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
