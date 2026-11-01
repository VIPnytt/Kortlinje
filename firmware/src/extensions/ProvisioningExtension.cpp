#include "extensions/ProvisioningExtension.h"

#include "config/secrets.h"
#include "extensions/AlarmExtension.h"
#include "fonts/SmallFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/WebServerService.h"

#include <ArduinoJson.h>
#include <array>
#include <HTTPClient.h>
#include <WiFi.h>
#include <cstdio>
#include <cstring>
#include <cyw43.h>
#include <lwip/apps/sntp.h>
#include <lwip/dhcp.h>
#include <lwip/netif.h>
#include <pico/unique_id.h>
#include <time.h>

namespace
{
// SoftAP glyph, 7×8, bit 6 at the left. Bottom dot stays on; arcs build then clear.
constexpr uint8_t wifiGlyphCols{7U};
constexpr uint8_t wifiGlyphRows{8U};
constexpr unsigned long wifiFrameMs{500UL};
constexpr std::array<std::array<uint8_t, wifiGlyphRows>, 3U> wifiFrames{{
    {
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0001000U,
    },
    {
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0000000U,
        0b0011100U,
        0b0100010U,
        0b0000000U,
        0b0001000U,
    },
    {
        0b0011100U,
        0b0111110U,
        0b1000001U,
        0b0000000U,
        0b0011100U,
        0b0100010U,
        0b0000000U,
        0b0001000U,
    },
}};

void drawWifiGlyph(int16_t x, int16_t y, uint8_t frame)
{
    const std::array<uint8_t, wifiGlyphRows> &rows{wifiFrames[frame]};
    for (uint8_t col{0U}; col < wifiGlyphCols; ++col)
    {
        for (uint8_t row{0U}; row < wifiGlyphRows; ++row)
        {
            if (((rows[row] >> (wifiGlyphCols - 1U - col)) & 0b1U) == 0U)
            {
                continue;
            }
            const int16_t pixelX{static_cast<int16_t>(x + col)};
            const int16_t pixelY{static_cast<int16_t>(y + row)};
            if (pixelX < 0 || pixelY < 0 || pixelX >= GRID_COLUMNS || pixelY >= GRID_ROWS)
            {
                continue;
            }
            DisplayService::setPixel(static_cast<uint8_t>(pixelX), static_cast<uint8_t>(pixelY), true);
        }
    }
}

constexpr const char *portalHead =
    "<!doctype html><html><head><meta charset=utf-8><meta name=viewport "
    "content=\"width=device-width,initial-scale=1\"><title>Kortlinje WiFi</title>"
    "<style>body{font:16px/1.4 system-ui,sans-serif;margin:16px;background:#111;color:#eee}"
    "button,select,input{font:inherit;width:100%;margin:6px 0;padding:10px;box-sizing:border-box}"
    "button{background:#0a7;color:#fff;border:0;border-radius:6px}</style></head><body>"
    "<h1>WiFi setup</h1><p>Pick a network and enter the password.</p>"
    "<form method=POST action=/portal/connect>"
    "<label>Network</label><select name=ssid>";

constexpr const char *portalTail =
    "</select>"
    "<label>Or type SSID</label><input name=manual placeholder=\"Hidden / manual\" autocomplete=off>"
    "<label>Password</label><input name=key type=password>"
    "<button type=submit>Connect</button>"
    "</form></body></html>";

constexpr const char *portalDoneHtml =
    "<!doctype html><html><head><meta charset=utf-8><meta name=viewport "
    "content=\"width=device-width,initial-scale=1\"><title>Saved</title>"
    "<style>body{font:16px/1.4 system-ui,sans-serif;margin:16px;background:#111;color:#eee}</style>"
    "</head><body><h1>Saved</h1><p>Clock is rebooting to join WiFi…</p></body></html>";

void appendEscaped(String &out, const char *text)
{
    for (const char *p{text}; p != nullptr && *p != '\0'; ++p)
    {
        switch (*p)
        {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out += *p;
            break;
        }
    }
}

String buildPortalHtml()
{
    // Embed SSIDs in HTML — captive mini-browsers often block fetch()/XHR.
    String html{portalHead};
    html += "<option value=\"\">— select —</option>";
    for (const std::string &ssid : ProvisioningExtension::scannedNetworksPublic())
    {
        html += "<option value=\"";
        appendEscaped(html, ssid.c_str());
        html += "\">";
        appendEscaped(html, ssid.c_str());
        html += "</option>";
    }
    html += portalTail;
    return html;
}
} // namespace

void ProvisioningExtension::begin()
{
    registerRoutes();
    transmit();
    if (state == State::ApWait)
    {
        // SoftAP already requested before begin (boot path)
        return;
    }
}

void ProvisioningExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (!payload["clear"].is<bool>() || !payload["clear"].as<bool>())
    {
        return;
    }
    if (!clearCredentials())
    {
        Serial.println("[provision] failed to clear WiFi");
        return;
    }
    Serial.println("[provision] cleared WiFi via Web UI — reboot");
    transmit();
    statusMs = millis();
    rebootPending = true;
}

void ProvisioningExtension::transmit()
{
    JsonDocument doc{};
    std::string ssid{};
    std::string key{};
    const bool configured{loadCredentials(ssid, key) || secretsFallback(ssid, key)};
    doc["configured"].set(configured);
    doc["ssid"].set(configured ? ssid.c_str() : "");
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}

bool ProvisioningExtension::clearCredentials()
{
    // remove() is false both when the file is already gone and when deletion fails.
    std::string remaining{};
    if ((!ConfigHandler::remove(configModule, "ssid") && ConfigHandler::get(configModule, "ssid", remaining)) ||
        (!ConfigHandler::remove(configModule, "key") && ConfigHandler::get(configModule, "key", remaining)))
    {
        return false;
    }
    // Suppress secrets.h WIFI_SSID fallback until the user saves new portal creds.
    return ConfigHandler::set(configModule, "forgotten", true);
}

void ProvisioningExtension::handle()
{
    if (enterRequested)
    {
        enterRequested = false;
        if (state != State::ApWait)
        {
            sntp_stop();
            // Leave STA without WiFi.disconnect() — that demotes AP_STA→AP and breaks later join.
            cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
            scanAndCache();
            startAp();
            Serial.printf("[provision] SoftAP %s result-state=%u networks=%u\n",
                          apName.data(),
                          static_cast<unsigned>(state),
                          static_cast<unsigned>(scannedNetworks.size()));
        }
    }

    if (dnsRunning)
    {
        handleDns();
    }

    pollNtp();

    if (rebootPending && millis() - statusMs >= rebootDelayMs)
    {
        Serial.println("[provision] reboot → STA join");
        stopDns();
        WiFi.mode(WIFI_AP_STA);
        static_cast<void>(WiFi.disconnectAP());
        rp2040.reboot();
    }

    // Boot had creds but DHCP/IP not ready yet — wait; only re-join if link really dropped.
    if (staRetryPending && state == State::Idle)
    {
        if (stationOnline())
        {
            staRetryPending = false;
            requestNtp();
        }
        else
        {
            const int link{cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA)};
            const bool associated{link == CYW43_LINK_JOIN || link == CYW43_LINK_NOIP || link == CYW43_LINK_UP};
            const unsigned long gap{associated ? staRetryMs : staRejoinMs};
            if (millis() - statusMs >= gap)
            {
                if (associated)
                {
                    // Still associated — keep waiting for DHCP; do not tear down.
                    statusMs = millis();
                    kickStaDhcp();
                    Serial.printf("[provision] waiting DHCP link=%d ip=%s\n",
                                  link,
                                  WiFi.localIP().toString().c_str());
                    requestNtp();
                }
                else
                {
                    std::string ssid{};
                    std::string key{};
                    if (loadCredentials(ssid, key) || secretsFallback(ssid, key))
                    {
                        Serial.printf("[provision] background STA rejoin link=%d\n", link);
                        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
                        joinAttempts = 0U;
                        connectStation(ssid, key);
                        // connectStation blocks up to setTimeout — measure gap from its end.
                        statusMs = millis();
                        requestNtp();
                    }
                    else
                    {
                        staRetryPending = false;
                    }
                }
            }
        }
    }

    if (state == State::Idle)
    {
        return;
    }

    if (state == State::ApWait && timeoutMs != 0UL && millis() - enteredMs >= timeoutMs)
    {
        std::string ssid{};
        std::string key{};
        if (loadCredentials(ssid, key) || secretsFallback(ssid, key))
        {
            exit();
            return;
        }
        // No fallback network — keep the captive portal up.
        enteredMs = millis();
    }

    if (state == State::Joining)
    {
        if (stationOnline())
        {
            finishOk();
        }
        else if (millis() - statusMs >= joinAttemptMs)
        {
            if (joinAttempts < maxJoinAttempts)
            {
                Serial.printf("[provision] join retry %u status=%d ip=%s\n",
                              static_cast<unsigned>(joinAttempts + 1U),
                              static_cast<int>(WiFi.status()),
                              WiFi.localIP().toString().c_str());
                connectStation(pendingSsid, pendingKey);
            }
            else
            {
                finishFail();
            }
        }
    }
    else if (state == State::Ok || state == State::Fail)
    {
        if (rebootPending)
        {
            return;
        }
        if (millis() - statusMs >= 4000UL)
        {
            if (state == State::Fail)
            {
                startAp();
            }
            else
            {
                state = State::Idle;
            }
        }
    }
}

bool ProvisioningExtension::isActive()
{
    return state == State::ApWait || state == State::Joining || state == State::Ok || state == State::Fail;
}

void ProvisioningExtension::toggle()
{
    if (isActive())
    {
        exit();
    }
    else
    {
        enter();
    }
}

void ProvisioningExtension::enter()
{
    if (state == State::ApWait || enterRequested)
    {
        return;
    }
    // Defer WiFi bring-up out of the button ISR/handler path.
    enterRequested = true;
    Serial.println("[provision] enter requested");
}

void ProvisioningExtension::exit()
{
    if (state == State::Idle)
    {
        return;
    }
    stopAp();
    state = State::Idle;
    std::string ssid{};
    std::string key{};
    if (loadCredentials(ssid, key) || secretsFallback(ssid, key))
    {
        connectStation(ssid, key);
        requestNtp();
    }
}

bool ProvisioningExtension::stationOnline()
{
    const IPAddress ip{WiFi.localIP()};
    if (!ip.isSet())
    {
        return false;
    }
    const int link{cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA)};
    // After DHCP, cyw43 often reports LINK_UP; arduino-pico WiFi.connected() only accepts JOIN.
    return link == CYW43_LINK_JOIN || link == CYW43_LINK_NOIP || link == CYW43_LINK_UP;
}

void ProvisioningExtension::kickStaDhcp()
{
    for (struct netif *n{netif_list}; n != nullptr; n = n->next)
    {
        if (!netif_is_up(n))
        {
            netif_set_up(n);
        }
        if (!netif_is_link_up(n))
        {
            netif_set_link_up(n);
        }
        if (netif_dhcp_data(n) != nullptr)
        {
            static_cast<void>(dhcp_renew(n));
        }
        else
        {
            static_cast<void>(dhcp_start(n));
        }
    }
}

bool ProvisioningExtension::beginStation()
{
    std::string ssid{};
    std::string key{};
    if (!loadCredentials(ssid, key) && !secretsFallback(ssid, key))
    {
        // No saved WiFi — SoftAP only in this case (or later via SET hold).
        scanAndCache();
        startAp();
        return false;
    }

    // No pre-join scan when creds exist — scan costs seconds and isn't needed for STA join.
    WiFi.mode(WIFI_STA);

    // Associate, then only briefly wait for DHCP. Blocking too long in begin0 starves the
    // rest of boot; finish DHCP in handle() with kickStaDhcp() instead.
    joinAttempts = 0U;
    bool associated{false};
    for (uint8_t round{0U}; round < bootJoinAttempts; ++round)
    {
        if (round > 0U)
        {
            const int link{cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA)};
            if (link == CYW43_LINK_JOIN || link == CYW43_LINK_NOIP || link == CYW43_LINK_UP)
            {
                associated = true;
                break;
            }
            Serial.printf("[provision] boot STA rejoin %u (link=%d)\n",
                          static_cast<unsigned>(round + 1U),
                          link);
            cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
            delay(100);
        }
        connectStation(ssid, key);
        kickStaDhcp();
        const unsigned long start{millis()};
        unsigned long associatedMs{0UL};
        while (millis() - start < bootJoinWaitMs)
        {
            yield();
            if (stationOnline())
            {
                Serial.printf("[provision] STA ok ip=%s link=%d\n",
                              WiFi.localIP().toString().c_str(),
                              cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA));
                staRetryPending = false;
                return true;
            }
            const int link{cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA)};
            if (link == CYW43_LINK_JOIN || link == CYW43_LINK_NOIP || link == CYW43_LINK_UP)
            {
                if (!associated)
                {
                    kickStaDhcp();
                }
                associated = true;
                associatedMs = millis();
            }
            else if (link == CYW43_LINK_FAIL || link == CYW43_LINK_BADAUTH || link == CYW43_LINK_NONET)
            {
                if (associatedMs == 0UL || millis() - associatedMs >= 1000UL)
                {
                    associated = false;
                    Serial.printf("[provision] link=%d status=%d — rejoin\n",
                                  link,
                                  static_cast<int>(WiFi.status()));
                    break;
                }
            }
            delay(20);
        }
        if (stationOnline())
        {
            staRetryPending = false;
            return true;
        }
        Serial.printf("[provision] boot round %u link=%d status=%d ip=%s\n",
                      static_cast<unsigned>(round + 1U),
                      cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA),
                      static_cast<int>(WiFi.status()),
                      WiFi.localIP().toString().c_str());
        if (associated)
        {
            break;
        }
    }

    Serial.printf("[provision] STA %s — wait in background (no SoftAP)\n",
                  associated ? "associated, DHCP pending" : "not up yet");
    if (associated)
    {
        kickStaDhcp();
    }
    staRetryPending = true;
    statusMs = millis();
    requestNtp();
    return false;
}

void ProvisioningExtension::scanAndCache()
{
    scannedNetworks.clear();
    WiFi.mode(WIFI_STA);
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    delay(50);
    const int8_t n{WiFi.scanNetworks(false)};
    Serial.printf("[provision] STA scan -> %d\n", static_cast<int>(n));
    for (int8_t i{0}; i < n; ++i)
    {
        const char *ssid{WiFi.SSID(static_cast<uint8_t>(i))};
        if (ssid == nullptr || ssid[0] == '\0')
        {
            continue;
        }
        // De-dupe while preserving first (strongest) hit order from the driver.
        bool exists{false};
        for (const std::string &known : scannedNetworks)
        {
            if (known == ssid)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            scannedNetworks.emplace_back(ssid);
        }
    }
    WiFi.scanDelete();
}

void ProvisioningExtension::startAp()
{
    buildApName();
    // Must stay WIFI_AP_STA: WiFi.disconnect() demotes AP_STA→AP, after which
    // softAPdisconnect() won't call disconnectAP() and STA join fails.
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    const uint8_t result{WiFi.beginAP(apName.data(), nullptr, apChannel)};
    state = State::ApWait;
    enteredMs = millis();
    statusMs = enteredMs;
    DisplayService::setPower(true);
    // DNS before :80 rebind — phones probe captive.* within ~1s of DHCP.
    startDns();
    // beginAP / mode switch invalidates the previous listen socket — rebind :80 on the AP netif.
    WebServerService::restart();
    Serial.printf("[provision] beginAP(%s) -> %u mode=%u ip=%s dns=53\n",
                  apName.data(),
                  static_cast<unsigned>(result),
                  static_cast<unsigned>(WiFi.getMode()),
                  WiFi.softAPIP().toString().c_str());
    if (result != WL_CONNECTED)
    {
        state = State::Fail;
        statusMs = millis();
    }
}

void ProvisioningExtension::stopAp()
{
    stopDns();
    // Always disconnectAP — softAPdisconnect() only does that in AP_STA; once begin()
    // flips mode to STA it would call disconnect() and kill a live station link.
    static_cast<void>(WiFi.disconnectAP());
    WiFi.mode(WIFI_STA);
}

void ProvisioningExtension::startDns()
{
    stopDns();
    if (dnsUdp.begin(53) != 0)
    {
        dnsRunning = true;
        Serial.println("[provision] DNS hijack :53");
    }
    else
    {
        Serial.println("[provision] DNS bind failed");
    }
}

void ProvisioningExtension::stopDns()
{
    if (dnsRunning)
    {
        dnsUdp.stop();
        dnsRunning = false;
    }
}

void ProvisioningExtension::handleDns()
{
    // DHCP points DNS at 192.168.4.1. Modern clients append EDNS OPT in Additional —
    // echoing the whole packet then appending an Answer breaks section order and the
    // phone ignores us (no captive sheet). Only echo header + question; drop EDNS.
    const IPAddress apIp{WiFi.softAPIP()};
    for (int n{0}; n < 8; ++n)
    {
        const int packetSize{dnsUdp.parsePacket()};
        if (packetSize < 12)
        {
            return;
        }

        uint8_t req[512]{};
        const int len{dnsUdp.read(req, sizeof(req))};
        if (len < 12)
        {
            continue;
        }

        // Walk first QNAME (labels) + QTYPE/QCLASS.
        int qend{12};
        while (qend < len && req[qend] != 0)
        {
            const int lab{static_cast<int>(req[qend])};
            if ((lab & 0xC0) == 0xC0)
            {
                qend += 2; // compression pointer in question (rare)
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
        if (qend > len || qend + 16 > static_cast<int>(sizeof(req)))
        {
            continue;
        }

        const uint16_t qtype{static_cast<uint16_t>((req[qend - 4] << 8) | req[qend - 3])};

        uint8_t resp[512]{};
        std::memcpy(resp, req, static_cast<size_t>(qend));
        resp[2] = 0x84; // QR=1, AA=1
        resp[3] = 0x00; // RCODE=0 (no RA flag needed for authoritative)
        resp[4] = 0x00;
        resp[5] = 0x01; // QDCOUNT = 1
        resp[6] = 0x00;
        resp[7] = 0x00; // ANCOUNT
        resp[8] = 0x00;
        resp[9] = 0x00; // NSCOUNT
        resp[10] = 0x00;
        resp[11] = 0x00; // ARCOUNT = 0 (drop EDNS)

        size_t pos{static_cast<size_t>(qend)};
        // A → SoftAP IP. AAAA/HTTPS/other → NODATA so the client falls through to A.
        if (qtype == 0x0001U)
        {
            resp[7] = 0x01;
            resp[pos++] = 0xC0;
            resp[pos++] = 0x0C;
            resp[pos++] = 0x00;
            resp[pos++] = 0x01; // TYPE A
            resp[pos++] = 0x00;
            resp[pos++] = 0x01; // CLASS IN
            resp[pos++] = 0x00;
            resp[pos++] = 0x00;
            resp[pos++] = 0x00;
            resp[pos++] = 0x3C; // TTL 60s
            resp[pos++] = 0x00;
            resp[pos++] = 0x04;
            resp[pos++] = apIp[0];
            resp[pos++] = apIp[1];
            resp[pos++] = apIp[2];
            resp[pos++] = apIp[3];
        }

        dnsUdp.beginPacket(dnsUdp.remoteIP(), dnsUdp.remotePort());
        dnsUdp.write(resp, pos);
        dnsUdp.endPacket();
    }
}

void ProvisioningExtension::connectStation(std::string_view ssid, std::string_view key)
{
    const std::string _ssid{ssid};
    const std::string _key{key};
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    // Slightly under bootJoinWaitMs so a failed join returns before we give up waiting.
    WiFi.setTimeout(4000);
    ++joinAttempts;
    statusMs = millis();
    // beginNoBlock still blocks inside CYW43::begin for up to setTimeout — expected.
    const int st{WiFi.beginNoBlock(_ssid.c_str(), _key.empty() ? nullptr : _key.c_str())};
    Serial.printf("[provision] beginNoBlock ssid='%s' keylen=%u attempt=%u -> %d connected=%d ip=%s\n",
                  _ssid.c_str(),
                  static_cast<unsigned>(_key.size()),
                  static_cast<unsigned>(joinAttempts),
                  st,
                  WiFi.connected() ? 1 : 0,
                  WiFi.localIP().toString().c_str());
}

void ProvisioningExtension::finishOk()
{
    static_cast<void>(saveCredentials(pendingSsid, pendingKey));
    state = State::Ok;
    statusMs = millis();
    requestNtp();
    pendingSsid.clear();
    pendingKey.clear();
    joinAttempts = 0U;
}

void ProvisioningExtension::requestNtp()
{
    ntpPending = true;
    ntpStarted = false;
    Serial.println("[provision] NTP pending");
}

void ProvisioningExtension::pollNtp()
{
    if (!ntpPending || !stationOnline())
    {
        return;
    }
    if (staRetryPending)
    {
        staRetryPending = false;
        Serial.printf("[provision] STA ok (bg) ip=%s link=%d\n",
                      WiFi.localIP().toString().c_str(),
                      cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA));
    }
    // Brief settle after SoftAP → STA / late join.
    if (!ntpStarted && millis() - statusMs < 300UL)
    {
        return;
    }

    if (!ntpStarted)
    {
        sntp_stop();
        NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
        // NTP.begin(hostname) sets running even when DNS fails and never starts SNTP.
        if (time(nullptr) < 10'000'000L)
        {
            NTP.begin(IPAddress(216, 239, 35, 0), IPAddress(162, 159, 200, 1));
        }
        ntpStarted = true;
        statusMs = millis();
        Serial.println("[provision] NTP started");
        return;
    }

    if (time(nullptr) >= 10'000'000L)
    {
        AlarmExtension::reschedule();
        Serial.println("[provision] NTP ok");
        ntpPending = false;
        ntpStarted = false;
        return;
    }

    if (millis() - statusMs >= 15000UL)
    {
        Serial.println("[provision] NTP timeout, retry");
        ntpStarted = false;
        statusMs = millis();
    }
}

void ProvisioningExtension::finishFail()
{
    state = State::Fail;
    statusMs = millis();
    joinAttempts = 0U;
    Serial.printf("[provision] join FAIL status=%d\n", static_cast<int>(WiFi.status()));
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
}

void ProvisioningExtension::buildApName()
{
    pico_unique_board_id_t board{};
    pico_get_unique_board_id(&board);
    snprintf(apName.data(),
             apName.size(),
             "K%02X%02X",
             board.id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES - 2U],
             board.id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES - 1U]);
}

bool ProvisioningExtension::loadCredentials(std::string &ssid, std::string &key)
{
    std::string _ssid{};
    std::string _key{};
    if (!ConfigHandler::get(configModule, "ssid", _ssid) || _ssid.empty())
    {
        return false;
    }
    ConfigHandler::get(configModule, "key", _key);
    ssid = std::move(_ssid);
    key = std::move(_key);
    return true;
}

bool ProvisioningExtension::saveCredentials(std::string_view ssid, std::string_view key)
{
    if (!ConfigHandler::set(configModule, "ssid", ssid) || !ConfigHandler::set(configModule, "key", key))
    {
        return false;
    }
    static_cast<void>(ConfigHandler::remove(configModule, "forgotten"));
    return true;
}

bool ProvisioningExtension::secretsFallback(std::string &ssid, std::string &key)
{
    bool forgotten{false};
    if (ConfigHandler::get(configModule, "forgotten", forgotten) && forgotten)
    {
        return false;
    }
#ifdef WIFI_SSID
    const char *compiled{WIFI_SSID};
    if (compiled == nullptr || compiled[0] == '\0')
    {
        return false;
    }
    ssid = compiled;
#ifdef WIFI_KEY
    key = WIFI_KEY;
#else
    key.clear();
#endif
    return true;
#else
    static_cast<void>(ssid);
    static_cast<void>(key);
    return false;
#endif
}

void ProvisioningExtension::draw()
{
    // SmallFont is ~7px tall on a 9px panel — only one line fits without overlap.
    DisplayService::fillFrame(false);
    SmallFont font{};
    std::string line{};
    switch (state)
    {
    case State::Joining:
        line = "...";
        break;
    case State::Ok:
        if (stationOnline())
        {
            const IPAddress ip{WiFi.localIP()};
            char buf[8]{};
            std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(ip[3]));
            line = buf;
        }
        else
        {
            line = "OK";
        }
        break;
    case State::Fail:
        line = "ERR";
        break;
    default:
    {
        // Phone SSID stays apName. Glyph: dot, then middle arc, then top arc, then both arcs off.
        static uint8_t frame{0U};
        static unsigned long frameMs{0UL};
        const unsigned long now{millis()};
        if (frameMs == 0UL || now - frameMs >= wifiFrameMs)
        {
            if (frameMs != 0UL)
            {
                frame = static_cast<uint8_t>((frame + 1U) % wifiFrames.size());
            }
            frameMs = now;
        }
        TextHandler ssid{std::string{apName.data()}, font};
        const uint8_t textHeight{ssid.getHeight()};
        const uint8_t lineHeight{textHeight > wifiGlyphRows ? textHeight : wifiGlyphRows};
        const int16_t lineY{static_cast<int16_t>((GRID_ROWS - lineHeight) / 2U)};
        constexpr uint8_t tracking{1U};
        const uint16_t totalWidth{static_cast<uint16_t>(wifiGlyphCols + tracking + ssid.getWidth())};
        const int16_t originX{static_cast<int16_t>(totalWidth < GRID_COLUMNS ? (GRID_COLUMNS - totalWidth) / 2U : 0U)};
        drawWifiGlyph(originX, static_cast<int16_t>(lineY + (lineHeight - wifiGlyphRows)), frame);
        ssid.draw(static_cast<int16_t>(originX + wifiGlyphCols + tracking),
                  static_cast<int8_t>(lineY + (lineHeight - textHeight)));
        return;
    }
    }

    TextHandler text{std::move(line), font};
    text.draw();
}

void ProvisioningExtension::registerRoutes()
{
    using Method = AsyncWebRequestMethod::AsyncWebRequestMethodType;
    WebServerService::http.on("/portal", Method::HTTP_GET, &onGetPortal);
    WebServerService::http.on("/portal/", Method::HTTP_GET, &onGetPortal);
    WebServerService::http.on("/portal/scan", Method::HTTP_GET, &onGetScan);
    WebServerService::http.on("/portal/connect", Method::HTTP_POST, &onPostConnect);
    WebServerService::http.on("/generate_204", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/gen_204", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/hotspot-detect.html", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/library/test/success.html", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/connecttest.txt", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/ncsi.txt", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/canonical.html", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/success.txt", Method::HTTP_GET, &servePortal);
    WebServerService::http.on("/fwlink", Method::HTTP_GET, &servePortal);
}

void ProvisioningExtension::onGetPortal(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
        return;
    }
    AsyncWebServerResponse *response{
        request->beginResponse(t_http_codes::HTTP_CODE_OK, "text/html", buildPortalHtml())};
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate", false);
    response->addHeader("Pragma", "no-cache", false);
    request->send(response);
}

void ProvisioningExtension::onGetScan(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
        return;
    }

    // Return the pre-SoftAP cache only — live scan while AP is up fails on CYW43.
    AsyncResponseStream *response{request->beginResponseStream("application/json")};
    response->print("{\"networks\":[");
    bool first{true};
    for (const std::string &ssid : scannedNetworks)
    {
        if (!first)
        {
            response->print(',');
        }
        first = false;
        response->print('"');
        for (const char ch : ssid)
        {
            if (ch == '"' || ch == '\\')
            {
                response->print('\\');
            }
            response->print(ch);
        }
        response->print('"');
    }
    response->print("]}");
    request->send(response);
}

void ProvisioningExtension::onPostConnect(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
        return;
    }

    String manual{};
    String selected{};
    if (request->hasParam("manual", true))
    {
        manual = request->getParam("manual", true)->value();
        manual.trim();
    }
    if (request->hasParam("ssid", true))
    {
        selected = request->getParam("ssid", true)->value();
        selected.trim();
    }
    pendingSsid = !manual.isEmpty() ? manual.c_str() : selected.c_str();
    pendingKey = request->hasParam("key", true) ? request->getParam("key", true)->value().c_str() : "";
    if (pendingSsid.empty())
    {
        request->send(t_http_codes::HTTP_CODE_BAD_REQUEST, "text/html",
                      "<p>SSID required. <a href=/portal>Back</a></p>");
        return;
    }

    // SoftAP→STA join on CYW43 can hang forever inside CYW43::begin (link up, join never
    // finishes) — display stuck on "...". Save creds, stop AP, reboot; cold boot joins cleanly.
    if (!saveCredentials(pendingSsid, pendingKey))
    {
        request->send(t_http_codes::HTTP_CODE_INTERNAL_SERVER_ERROR, "text/html",
                      "<p>Save failed. <a href=/portal>Back</a></p>");
        return;
    }
    Serial.printf("[provision] saved ssid='%s', will reboot\n", pendingSsid.c_str());
    pendingSsid.clear();
    pendingKey.clear();
    // Keep SoftAP up until reboot so the phone still receives "Saved".
    state = State::Ok;
    statusMs = millis();
    rebootPending = true;
    request->send(t_http_codes::HTTP_CODE_OK, "text/html", portalDoneHtml);
}

void ProvisioningExtension::servePortal(AsyncWebServerRequest *request)
{
    onGetPortal(request);
}

bool ProvisioningExtension::handleNotFound(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        return false;
    }
    // Any unknown host/path (after DNS hijack) opens the portal — no URL typing.
    onGetPortal(request);
    return true;
}
