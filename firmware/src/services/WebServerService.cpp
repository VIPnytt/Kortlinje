#include "services/WebServerService.h"

#include "services/DeviceService.h"

#include <HTTPClient.h>
#include <WiFi.h>

void WebServerService::begin()
{
    snprintf(etag.data(), etag.size(), "\"%08X\"", calculateHash(INDEX_HTML_VIEW));
    http.on("/", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_GET, &onGetRoot);
    http.on("/", AsyncWebRequestMethod::AsyncWebRequestMethodType::HTTP_HEAD, &onHeadRoot);
    http.onNotFound(&onNotFound);
    http.begin();
    socket->onEvent(&onEvent);
    WebServer.http.addHandler(socket);
}

void WebServerService::handle() { socket->cleanupClients(); }

void WebServerService::onEmpty(AsyncWebServerRequest *request) {}

void WebServerService::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg,
                               const uint8_t *data, size_t len)
{
    switch (type)
    {
    case AwsEventType::WS_EVT_CONNECT:
    {
        const JsonObjectConst transmits{Device.getTransmits()};
        const size_t length{measureJson(transmits)};
        std::vector<char> message(length + 1U);
        serializeJson(transmits, message.data(), length + 1U);
        client->text(message.data(), length);
#if EXTENSION_WEBAPP
        WebAppExtension::render = true;
#endif // EXTENSION_WEBAPP
    }
    break;
    case AwsEventType::WS_EVT_DATA:
    {
        const AwsFrameInfo *info{static_cast<AwsFrameInfo *>(arg)};
        if (info->message_opcode == AwsFrameType::WS_TEXT)
        {
            if (info->final == 0U || info->index != 0U || info->index + len != info->len)
            {
                if (info->num == 0U && info->index == 0U)
                {
                    buffer.clear();
                }
                const std::span<const uint8_t> chunk{data, len};
                buffer.insert(buffer.end(), chunk.begin(), chunk.end());
                if (info->final == 0U || info->index + len != info->len)
                {
                    return;
                }
                data = buffer.data();
                len = buffer.size();
            }
            JsonDocument doc{};
            if (deserializeJson(doc, data, len) == DeserializationError::Code::Ok && doc.is<JsonObjectConst>())
            {
                for (const JsonPairConst pair : doc.as<JsonObjectConst>())
                {
                    if (pair.value().is<JsonObjectConst>())
                    {
                        Device.receive(pair.value().as<JsonObjectConst>(),
                                       name,
                                       std::string_view(pair.key().c_str(), pair.key().size()));
                    }
                }
            }
        }
    }
    break;
    default:
        break;
    }
}

void WebServerService::onHeadRoot(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_OK)};
    response->addHeader("Access-Control-Allow-Methods", "HEAD,GET", false);
    response->addHeader("Access-Control-Allow-Origin", "*", false);
    response->addHeader("Content-Encoding", "gzip", false);
    response->addHeader("ETag", etag.data(), false);
    response->setContentLength(length);
    request->send(response);
}

void WebServerService::onGetRoot(AsyncWebServerRequest *request)
{
    const AsyncWebHeader *_etag{request->getHeader("If-None-Match")};
    if (_etag != nullptr && strcmp(_etag->value().c_str(), etag.data()) == 0)
    {
        AsyncWebServerResponse *response{request->beginResponse(t_http_codes::HTTP_CODE_NOT_MODIFIED)};
        response->addHeader("ETag", etag.data(), false);
        response->setContentLength(length);
        request->send(response);
    }
    else
    {
        AsyncWebServerResponse *response{
            request->beginResponse(200, "text/html", INDEX_HTML_VIEW.data(), INDEX_HTML_VIEW.size())};
        response->addHeader("Content-Encoding", "gzip", false);
        response->addHeader("ETag", etag.data(), false);
        request->send(response);
    }
}

void WebServerService::onNotFound(AsyncWebServerRequest *request) { request->send(t_http_codes::HTTP_CODE_NOT_FOUND); }

void WebServerService::onTransmit(JsonObjectConst payload, std::string_view source)
{
    JsonDocument doc{};
    doc[source].set(payload);
    const size_t length{measureJson(doc)};
    std::vector<char> message(length + 1U);
    serializeJson(doc, message.data(), length + 1U);
    socket->textAll(message.data(), length);
}

WebServerService &WebServerService::getInstance()
{
    static WebServerService instance;
    return instance;
}

WebServerService &WebServer{WebServerService::getInstance()};
