#include "extensions/WebUiExtension.h"

#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"

#include <HTTPClient.h>

void WebUiExtension::begin()
{
    WebServer.http.on("/", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_HEAD, &onHeadRoot);
    WebServer.http.on("/", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetHtml);
    WebServer.http.on("/favicon.svg", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetSvg);
    WebServer.http.on("/index.css", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetCss);
    WebServer.http.on("/index.js", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetJavascript);
}

void WebUiExtension::handle()
{
    if (render && pending && Extensions.webSocket().server->count() != 0U)
    {
        pending = false;
        transmit();
        alarm = add_alarm_in_ms(interval, &onAlarm, nullptr, true);
    }
}

int64_t WebUiExtension::onAlarm(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<void>(user_data);
    alarm = 0L;
    pending = true;
    return 0LL;
}

void WebUiExtension::throttle()
{
    if (interval != 0xFFU)
    {
        ++interval;
    }
}

void WebUiExtension::transmit()
{
    std::array<bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> frame{};
    Display.getFrame(frame);
    render = false;
    std::array<uint8_t, (frame.size() >> 3U)> bytes{};
    for (size_t idx{0U}; idx < frame.size(); ++idx)
    {
        if (frame[idx])
        {
            bytes[idx >> 3U] |= static_cast<uint8_t>(1U << (idx & 7U));
        }
    }
    JsonDocument doc{};
    JsonArray _frame{doc["frame"].to<JsonArray>()};
    for (const uint8_t byte : bytes)
    {
        _frame.add(byte);
    }
    Device.transmit(doc.as<JsonObjectConst>(), name, false);
}

void WebUiExtension::onGetCss(AsyncWebServerRequest *request)
{
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), cssEtag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", cssEtag.data(), false);
        response->setContentLength(css.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{request->beginResponse(200, "text/css", css.data(), css.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", cssEtag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onGetHtml(AsyncWebServerRequest *request)
{
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), htmlEtag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", htmlEtag.data(), false);
        response->setContentLength(html.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{request->beginResponse(200, "text/html", html.data(), html.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", htmlEtag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onGetJavascript(AsyncWebServerRequest *request)
{
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), javascriptEtag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", javascriptEtag.data(), false);
        response->setContentLength(javascript.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{
            request->beginResponse(200, "text/javascript", javascript.data(), javascript.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", javascriptEtag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onGetSvg(AsyncWebServerRequest *request)
{
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), svgEtag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", svgEtag.data(), false);
        response->setContentLength(svg.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{request->beginResponse(200, "image/svg+xml", svg.data(), svg.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", svgEtag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onHeadRoot(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_OK)};
    response->addHeader("Access-Control-Allow-Methods", "HEAD,GET", false);
    response->addHeader("Access-Control-Allow-Origin", "*", false);
    response->addHeader("Content-Encoding", "gzip", false);
    response->addHeader("ETag", htmlEtag.data(), false);
    response->setContentLength(html.size());
    request->send(response);
}
