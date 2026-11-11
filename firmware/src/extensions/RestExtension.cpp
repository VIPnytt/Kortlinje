#include "extensions/RestExtension.h"

#include "services/DeviceService.h"
#include "services/WebServerService.h"

#include <HTTPClient.h>
#include <algorithm>

/**
 * @brief Registers the RESTful GET and PATCH endpoint handlers.
 */
void RestExtension::begin()
{
    WebServerService::http.on(AsyncURIMatcher::dir("/rest/"), WebRequestMethod::HTTP_GET, &onGet);
    WebServerService::http.on(
        AsyncURIMatcher::dir("/rest/"), WebRequestMethod::HTTP_PATCH, &WebServerService::onEmpty, nullptr, &onPatch);
}

void RestExtension::onGet(AsyncWebServerRequest *request)
{
    const String &url{request->url()};
    const std::string_view module{url.c_str() + prefixLength, url.length() - prefixLength};
    const JsonObjectConst transmits{DeviceService::getTransmits()};
    const JsonVariantConst target{module.empty() ? JsonVariantConst{transmits} : transmits[module]};
    if (!target.isNull())
    {
        AsyncResponseStream *response{request->beginResponseStream("application/json")};
        response->setCode(t_http_codes::HTTP_CODE_OK);
        serializeJson(target, *response);
        request->send(response);
    }
    else
    {
        request->send(t_http_codes::HTTP_CODE_NO_CONTENT);
    }
}

void RestExtension::onPatch(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index, size_t total)
{
    const bool final{index + len == total};
    if (request->contentType() == "application/json")
    {
        if (index != 0U || len != total)
        {
            if (index == 0U)
            {
                buffer.resize(total);
            }
            if (index + len <= buffer.size())
            {
                std::copy_n(data, len, buffer.begin() + static_cast<std::ptrdiff_t>(index));
            }
            if (!final)
            {
                return;
            }
            data = buffer.data();
            len = buffer.size();
        }
        JsonDocument doc{};
        const DeserializationError error{deserializeJson(doc, data, len)};
        if (!buffer.empty())
        {
            buffer.clear();
            buffer.shrink_to_fit();
        }
        if (error == DeserializationError::Code::Ok && doc.is<JsonObjectConst>())
        {
            const String &url{request->url()};
            const std::string_view destination{url.c_str() + prefixLength, url.length() - prefixLength};
            if (!destination.empty())
            {
                DeviceService::receive(doc.as<JsonObjectConst>(), name, destination);
            }
            else
            {
                for (const JsonPairConst pair : doc.as<JsonObjectConst>())
                {
                    if (pair.value().is<JsonObjectConst>())
                    {
                        DeviceService::receive(pair.value().as<JsonObjectConst>(),
                                               name,
                                               std::string_view{pair.key().c_str(), pair.key().size()});
                    }
                }
            }
            request->send(t_http_codes::HTTP_CODE_NO_CONTENT);
            return;
        }
    }
    if (final)
    {
        request->send(t_http_codes::HTTP_CODE_BAD_REQUEST);
    }
}
