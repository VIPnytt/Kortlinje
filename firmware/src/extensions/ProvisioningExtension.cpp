#include "extensions/ProvisioningExtension.h"

#include "config/secrets.h"
#include "fonts/SmallFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"
#include "services/WebServerService.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <cstring>
#include <lwip/apps/sntp.h>
#include <pico/unique_id.h>
#include <time.h>

namespace
{
constexpr const char *portalHtml =
    "<!doctype html><html><head><meta charset=utf-8><meta name=viewport "
    "content=\"width=device-width,initial-scale=1\"><title>Kortlinje WiFi</title>"
    "<style>body{font:16px/1.4 system-ui,sans-serif;margin:16px;background:#111;color:#eee}"
    "button,select,input{font:inherit;width:100%;margin:6px 0;padding:10px;box-sizing:border-box}"
    "button{background:#0a7;color:#fff;border:0;border-radius:6px}a{color:#8cf}</style></head><body>"
    "<h1>WiFi setup</h1><p>Pick a network and enter the password.</p>"
    "<label>Network</label><select id=s></select>"
    "<label>Or SSID</label><input id=m placeholder=\"Hidden / manual\">"
    "<label>Password</label><input id=p type=password>"
    "<button id=go type=button>Connect</button>"
    "<p id=t></p><script>"
    "const s=document.getElementById('s'),m=document.getElementById('m'),p=document.getElementById('p'),"
    "t=document.getElementById('t');"
    "fetch('/portal/scan').then(r=>r.json()).then(j=>{"
    " (j.networks||[]).forEach(n=>{const o=document.createElement('option');o.value=n;o.textContent=n;s.appendChild(o);});"
    "}).catch(()=>{t.textContent='Scan failed — type SSID manually.';});"
    "document.getElementById('go').onclick=async()=>{"
    " const ssid=(m.value||s.value||'').trim(); if(!ssid){t.textContent='Choose a network';return;}"
    " t.textContent='Connecting…';"
    " const b=new URLSearchParams({ssid,key:p.value});"
    " const r=await fetch('/portal/connect',{method:'POST',headers:{'Content-Type':"
    "'application/x-www-form-urlencoded'},body:b});"
    " t.textContent=r.ok?'Saved. Watch the clock display.':'Failed — try again.';"
    "};"
    "</script></body></html>";
} // namespace

void ProvisioningExtension::begin()
{
    registerRoutes();
    if (state == State::ApWait)
    {
        // SoftAP already requested before begin (boot path)
        return;
    }
}

void ProvisioningExtension::handle()
{
    if (state == State::Idle)
    {
        return;
    }

    if (state == State::ApWait && timeoutMs != 0UL && millis() - enteredMs >= timeoutMs)
    {
        exit();
        return;
    }

    if (state == State::Joining)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            finishOk();
        }
        else if (millis() - statusMs >= 30000UL)
        {
            finishFail();
        }
    }
    else if (state == State::Ok || state == State::Fail)
    {
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
    if (state == State::ApWait)
    {
        return;
    }
    sntp_stop();
    WiFi.disconnect(true);
    startAp();
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
        NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
    }
}

bool ProvisioningExtension::beginStation()
{
    std::string ssid{};
    std::string key{};
    if (!loadCredentials(ssid, key) && !secretsFallback(ssid, key))
    {
        startAp();
        return false;
    }
    connectStation(ssid, key);
    return true;
}

void ProvisioningExtension::startAp()
{
    buildApName();
    // AP+STA so /portal/scan can run without tearing down the portal session.
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    const uint8_t result{WiFi.beginAP(apName.data(), nullptr, apChannel)};
    state = State::ApWait;
    enteredMs = millis();
    statusMs = enteredMs;
    DisplayService::setPower(true);
    if (result != WL_CONNECTED)
    {
        state = State::Fail;
        statusMs = millis();
    }
}

void ProvisioningExtension::stopAp()
{
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
}

void ProvisioningExtension::connectStation(std::string_view ssid, std::string_view key)
{
    const std::string _ssid{ssid};
    const std::string _key{key};
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(_ssid.c_str(), _key.c_str());
}

void ProvisioningExtension::finishOk()
{
    static_cast<void>(saveCredentials(pendingSsid, pendingKey));
    stopAp();
    state = State::Ok;
    statusMs = millis();
    NTP.begin("1.pool.ntp.org", "2.pool.ntp.org");
    pendingSsid.clear();
    pendingKey.clear();
}

void ProvisioningExtension::finishFail()
{
    state = State::Fail;
    statusMs = millis();
    WiFi.disconnect(true);
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
    return ConfigHandler::set(configModule, "ssid", ssid) && ConfigHandler::set(configModule, "key", key);
}

bool ProvisioningExtension::secretsFallback(std::string &ssid, std::string &key)
{
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
    DisplayService::fillFrame(false);
    SmallFont font{};
    const char *status{"AP"};
    switch (state)
    {
    case State::Joining:
        status = "...";
        break;
    case State::Ok:
        status = "OK";
        break;
    case State::Fail:
        status = "ERR";
        break;
    default:
        status = "AP";
        break;
    }

    TextHandler statusText{status, font};
    statusText.draw(1, 1);

    if (state == State::ApWait || state == State::Fail)
    {
        TextHandler nameText{apName.data(), font};
        const int16_t x{static_cast<int16_t>((GRID_COLUMNS - nameText.getWidth()) / 2)};
        nameText.draw(x, static_cast<int8_t>(GRID_ROWS - nameText.getHeight()));
    }
    else if (state == State::Ok && WiFi.status() == WL_CONNECTED)
    {
        const IPAddress ip{WiFi.localIP()};
        char buf[8]{};
        snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(ip[3]));
        TextHandler ipText{buf, font};
        const int16_t x{static_cast<int16_t>((GRID_COLUMNS - ipText.getWidth()) / 2)};
        ipText.draw(x, static_cast<int8_t>(GRID_ROWS - ipText.getHeight()));
    }
}

void ProvisioningExtension::registerRoutes()
{
    using Method = AsyncWebRequestMethod::AsyncWebRequestMethodType;
    WebServerService::http.on("/portal", Method::HTTP_GET, &onGetPortal);
    WebServerService::http.on("/portal/scan", Method::HTTP_GET, &onGetScan);
    WebServerService::http.on("/portal/connect", Method::HTTP_POST, &onPostConnect);
    WebServerService::http.on("/generate_204", Method::HTTP_GET, &redirectPortal);
    WebServerService::http.on("/gen_204", Method::HTTP_GET, &redirectPortal);
    WebServerService::http.on("/hotspot-detect.html", Method::HTTP_GET, &redirectPortal);
    WebServerService::http.on("/connecttest.txt", Method::HTTP_GET, &redirectPortal);
    WebServerService::http.on("/ncsi.txt", Method::HTTP_GET, &redirectPortal);
    WebServerService::http.on("/canonical.html", Method::HTTP_GET, &redirectPortal);
}

void ProvisioningExtension::onGetPortal(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
        return;
    }
    request->send(t_http_codes::HTTP_CODE_OK, "text/html", portalHtml);
}

void ProvisioningExtension::onGetScan(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
        return;
    }

    AsyncResponseStream *response{request->beginResponseStream("application/json")};
    response->print("{\"networks\":[");

    const int8_t n{WiFi.scanNetworks(false)};
    bool first{true};
    for (int8_t i{0}; i < n; ++i)
    {
        const String ssid{WiFi.SSID(static_cast<uint8_t>(i))};
        if (ssid.isEmpty())
        {
            continue;
        }
        if (!first)
        {
            response->print(',');
        }
        first = false;
        response->print('"');
        for (size_t c{0}; c < ssid.length(); ++c)
        {
            const char ch{ssid.charAt(c)};
            if (ch == '"' || ch == '\\')
            {
                response->print('\\');
            }
            response->print(ch);
        }
        response->print('"');
    }
    WiFi.scanDelete();

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
    if (!request->hasParam("ssid", true))
    {
        request->send(t_http_codes::HTTP_CODE_BAD_REQUEST, "text/plain", "ssid required");
        return;
    }
    pendingSsid = request->getParam("ssid", true)->value().c_str();
    pendingKey = request->hasParam("key", true) ? request->getParam("key", true)->value().c_str() : "";
    if (pendingSsid.empty())
    {
        request->send(t_http_codes::HTTP_CODE_BAD_REQUEST, "text/plain", "ssid required");
        return;
    }

    state = State::Joining;
    statusMs = millis();
    stopAp();
    connectStation(pendingSsid, pendingKey);
    request->send(t_http_codes::HTTP_CODE_OK, "text/plain", "ok");
}

void ProvisioningExtension::redirectPortal(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
        return;
    }
    request->redirect("http://192.168.4.1/portal");
}

bool ProvisioningExtension::handleNotFound(AsyncWebServerRequest *request)
{
    if (!isActive())
    {
        return false;
    }
    redirectPortal(request);
    return true;
}
