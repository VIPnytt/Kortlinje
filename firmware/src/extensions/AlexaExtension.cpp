#include "extensions/AlexaExtension.h"

#include "constants/Kortlinje.h"
#include "services/DisplayService.h"
#include "services/WebServerService.h"

#include <HTTPClient.h>
#include <SimpleMDNS.h>
#include <WiFi.h>
#include <algorithm>
#include <format>

/**
 * @brief Initializes the Alexa-compatible HTTP and UPnP services.
 */
void AlexaExtension::begin()
{
    WebServerService::http.on(
        AsyncURIMatcher::exact("/api"), WebRequestMethod::HTTP_POST, &WebServerService::onEmpty, nullptr, &onPostApi);
    WebServerService::http.on(AsyncURIMatcher::exact("/api/alexa/lights"), WebRequestMethod::HTTP_GET, &onGetLights);
    WebServerService::http.on(AsyncURIMatcher::exact("/api/alexa/lights/1"), WebRequestMethod::HTTP_GET, &onGetLight);
    WebServerService::http.on(AsyncURIMatcher::exact("/api/alexa/lights/1/state"),
                              WebRequestMethod::HTTP_PUT,
                              &WebServerService::onEmpty,
                              nullptr,
                              &onPutState);
    WebServerService::http.on(
        AsyncURIMatcher::exact("/description.xml"), WebRequestMethod::HTTP_GET, &onGetDescription);
    upnp.onPacket(&onUpnp);
}

std::array<char, 0b1U << 8U> AlexaExtension::light()
{
    std::array<char, 0b1U << 8U> buffer{};
    std::format_to_n(buffer.begin(),
                     buffer.size() - 1U,
                     R"({{"1":{{)"
                     R"("manufacturername":"IKEA",)"
                     R"("modelid":"Kortlinje",)"
                     R"("name":"{}",)"
                     R"("state":{{)"
                     R"("bri":{},)"
                     R"("mode":"homeautomation",)"
                     R"("on":{},)"
                     R"("reachable":true)"
                     R"(}},)"
                     R"("swversion":"{}",)"
                     R"("type":"Dimmable light",)"
                     R"("uniqueid":"00:{:012x}:00:00")"
                     R"(}}}})",
                     Kortlinje::device::name.data(),
                     ((static_cast<unsigned int>(DisplayService::getBrightness()) + 1U) * 254U) >> 3U,
                     DisplayService::getPower(),
                     Kortlinje::version,
                     mac());
    return buffer;
}

uint64_t AlexaExtension::mac()
{
    std::array<uint8_t, 6U> _mac{};
    WiFi.macAddress(_mac.data());
    return (static_cast<uint64_t>(_mac[0U]) << 40U) | (static_cast<uint64_t>(_mac[1U]) << 32U) |
           (static_cast<uint64_t>(_mac[2U]) << 24U) | (static_cast<uint64_t>(_mac[3U]) << 16U) |
           (static_cast<uint64_t>(_mac[4U]) << 8U) | static_cast<uint64_t>(_mac[5U]);
}

void AlexaExtension::onGetDescription(AsyncWebServerRequest *request)
{
    const uint64_t _mac{mac()};
    const IPAddress ip{WiFi.localIP()};
    std::array<char, 640U> response{};
    std::format_to_n(response.begin(),
                     response.size() - 1U,
                     R"(<?xml version="1.0" encoding="UTF-8"?>)"
                     R"(<root xmlns="urn:schemas-upnp-org:device-1-0">)"
                     "<device>"
                     "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
                     "<friendlyName>{} ({}.local)</friendlyName>"
                     "<manufacturer>Royal Philips Electronics</manufacturer>"
                     "<manufacturerURL>http://www.philips.com</manufacturerURL>"
                     "<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>"
                     "<modelName>Philips hue bridge 2015</modelName>"
                     "<modelNumber>BSB002</modelNumber>"
                     "<modelURL>http://www.meethue.com</modelURL>"
                     "<serialNumber>{:06X}FFFE{:06X}</serialNumber>"
                     "<UDN>uuid:2f402f80-da50-11e1-9b23-{:012x}</UDN>"
                     "</device>"
                     "<specVersion>"
                     "<major>1</major>"
                     "<minor>0</minor>"
                     "</specVersion>"
                     "<URLBase>http://{}.{}.{}.{}:80/</URLBase>"
                     "</root>",
                     Kortlinje::device::name.data(),
                     Kortlinje::device::hostname.data(),
                     _mac >> 24U,
                     _mac & 0xFFFFFFU,
                     _mac,
                     ip[0U],
                     ip[1U],
                     ip[2U],
                     ip[3U]);
    request->send(t_http_codes::HTTP_CODE_OK, "text/xml", response.data());
}

void AlexaExtension::onGetLight(AsyncWebServerRequest *request)
{
    std::array<char, 0b1U << 8U> response{light()};
    for (size_t idx{response.size() - 1U}; idx > 5U; --idx)
    {
        if (response[idx] == '}')
        {
            response[idx] = '\0';
            break;
        }
    }
    request->send(t_http_codes::HTTP_CODE_OK, "application/json", response.data() + 5U);
}

void AlexaExtension::onGetLights(AsyncWebServerRequest *request)
{
    request->send(t_http_codes::HTTP_CODE_OK, "application/json", light().data());
}

/**
 * @brief Advertises the Hue-compatible mDNS service and bridge metadata.
 */
void AlexaExtension::onMdns()
{
    upnp.listenMulticast(IPAddress(239U, 255U, 255U, 250U), 1'900U);
    if (const hMDNSService service{MDNS.addService("hue", "tcp", 80U)})
    {
        const uint64_t _mac{mac()};
        std::array<char, 17U> bridgeId{};
        std::format_to_n(bridgeId.begin(), bridgeId.size() - 1U, "{:06X}FFFE{:06X}", _mac >> 24U, _mac & 0xFFFFFFU);
        MDNS.addServiceTxt(service, "bridgeid", bridgeId.data());
        MDNS.addServiceTxt(service, "modelid", "BSB002");
    }
}

/**
 * @brief Applies brightness and power updates from a JSON request body.
 *
 * @param request HTTP request receiving the response.
 * @param data Current request body data segment.
 * @param len Length of the current body data segment.
 * @param index Offset of the current body data segment within the request body.
 * @param total Total request body length.
 */
void AlexaExtension::onPutState(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index,
                                size_t total)
{
    static_cast<void>(index);
    static_cast<void>(total);
    JsonDocument doc{};
    if (deserializeJson(doc, data, len) == DeserializationError::Code::Ok)
    {
        if (doc["bri"].is<uint8_t>())
        {
            const auto brightness{static_cast<DisplayService::Brightness>(doc["bri"].as<uint8_t>() >> 5U)};
            if (DisplayService::getBrightness() != brightness)
            {
                DisplayService::setBrightness(brightness);
            }
        }
        if (doc["on"].is<bool>())
        {
            const bool power{doc["on"].as<bool>()};
            if (DisplayService::getPower() != power)
            {
                DisplayService::setPower(power);
            }
        }
    }
    request->send(t_http_codes::HTTP_CODE_OK,
                  "application/json",
                  DisplayService::getPower() ? R"([{"success":{"/lights/1/state/on":true}}])"
                                             : R"([{"success":{"/lights/1/state/on":false}}])");
}

/**
 * @brief Handles API authentication requests with a successful Alexa user response.
 */
void AlexaExtension::onPostApi(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index,
                               size_t total)
{
    static_cast<void>(data);
    static_cast<void>(len);
    static_cast<void>(index);
    static_cast<void>(total);
    request->send(t_http_codes::HTTP_CODE_OK, "application/json", R"([{"success":{"username":"alexa"}}])");
}

/**
 * @brief Responds to Hue-compatible UPnP discovery requests.
 *
 * @param packet Received UDP packet containing the discovery request.
 */
void AlexaExtension::onUpnp(AsyncUDPPacket &packet)
{
    const std::span<const uint8_t> request{packet.data(), packet.length()};
    if (std::ranges::starts_with(request, std::string_view{"M-SEARCH"}))
    {
        const bool root{std::ranges::search(request, std::string_view{"upnp:rootdevice"}).begin() != request.end()};
        const uint64_t _mac{mac()};
        const IPAddress ip{WiFi.localIP()};
        std::array<uint8_t, 320U> response{};
        const auto [end, _]{std::format_to_n(response.begin(),
                                             response.size(),
                                             "HTTP/1.1 200 OK\r\n"
                                             "EXT:\r\n"
                                             "LOCATION: http://{}.{}.{}.{}:80/description.xml\r\n"
                                             "SERVER: FreeRTOS/6.0.5, UPnP/1.0, IpBridge/1.17.0\r\n"
                                             "ST: {}\r\n"
                                             "USN: uuid:2f402f80-da50-11e1-9b23-{:012x}{}\r\n"
                                             "hue-bridgeid: {:06X}FFFE{:06X}\r\n"
                                             "\r\n",
                                             ip[0U],
                                             ip[1U],
                                             ip[2U],
                                             ip[3U],
                                             root ? "upnp:rootdevice" : "urn:schemas-upnp-org:device:basic:1",
                                             _mac,
                                             root ? "::upnp:rootdevice" : "",
                                             _mac >> 24U,
                                             _mac & 0xFFFFFFU)};
        upnp.writeTo(
            response.data(), static_cast<size_t>(end - response.begin()), packet.remoteIP(), packet.remotePort());
    }
}
