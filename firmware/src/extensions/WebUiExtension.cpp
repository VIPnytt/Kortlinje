#include "extensions/WebUiExtension.h"

#include "extensions/WebSocketExtension.h"
#include "handlers/EmbedHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"

#include <HTTPClient.h>
#include <array>

void WebUiExtension::begin()
{
    WebServerService::http.on("/",
                              AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET |
                                  AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_HEAD,
                              &onHtml);
    WebServerService::http.on("/favicon.svg", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetSvg);
    WebServerService::http.on("/index.css", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetCss);
    WebServerService::http.on(
        "/index.js", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetJavascript);
}

void WebUiExtension::handle()
{
    if (render && pending && WebSocketExtension::server().count() != 0U)
    {
        pending = false;
        std::array<uint8_t, Display::grid::bytes> bytes{};
        const auto frame{DisplayService::getFrame()};
        render = false;
        for (size_t idx{0U}; idx < frame.size(); ++idx)
        {
            bytes[idx >> 3U] |= static_cast<uint8_t>(frame[idx]) << (idx & 7U);
        }
        if (WebSocketExtension::server().binaryAll(bytes.data(), bytes.size()) != AsyncWebSocket::SendStatus::ENQUEUED)
        {
            ++interval;
            render = true;
        }
        add_alarm_in_us(interval, &onAlarm, this, true);
    }
}

int64_t WebUiExtension::onAlarm(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<WebUiExtension *>(user_data)};
    self->pending = true;
    return 0LL;
}

void WebUiExtension::onGetCss(AsyncWebServerRequest *request)
{
    static constexpr auto content{EmbedHandler::embed<uint8_t>({
#embed "../../../webui/dist/index.css.gz"
    })};
    static constexpr std::array<char, 11U> etag{WebServerService::etag(content)};
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), etag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", etag.data(), false);
        response->setContentLength(content.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{request->beginResponse(200, "text/css", content.data(), content.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", etag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onGetJavascript(AsyncWebServerRequest *request)
{
    static constexpr auto content{EmbedHandler::embed<uint8_t>({
#embed "../../../webui/dist/index.js.gz"
    })};
    static constexpr std::array<char, 11U> etag{WebServerService::etag(content)};
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), etag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", etag.data(), false);
        response->setContentLength(content.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{
            request->beginResponse(200, "text/javascript", content.data(), content.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", etag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onGetSvg(AsyncWebServerRequest *request)
{
    static constexpr auto content{EmbedHandler::embed<uint8_t>({
#embed "../../../webui/dist/favicon.svg.gz"
    })};
    static constexpr std::array<char, 11U> etag{WebServerService::etag(content)};
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), etag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", etag.data(), false);
        response->setContentLength(content.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{request->beginResponse(200, "image/svg+xml", content.data(), content.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", etag.data(), false);
        request->send(response);
    }
}

void WebUiExtension::onHtml(AsyncWebServerRequest *request)
{
    static constexpr auto content{EmbedHandler::embed<uint8_t>({
#embed "../../../webui/dist/index.html.gz"
    })};
    static constexpr std::array<char, 11U> etag{WebServerService::etag(content)};
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), etag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", etag.data(), false);
        response->setContentLength(content.size());
        request->send(response);
    }
    else if (request->method() == HTTP_HEAD)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_OK)};
        response->addHeader("Access-Control-Allow-Methods", "HEAD,GET", false);
        response->addHeader("Access-Control-Allow-Origin", "*", false);
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", etag.data(), false);
        response->setContentLength(content.size());
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{request->beginResponse(200, "text/html", content.data(), content.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", etag.data(), false);
        request->send(response);
    }
}
