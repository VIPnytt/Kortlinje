#include "handlers/WeatherHandler.h"

#include "config/version.h"
#include "handlers/CertificateHandler.h"
#include "modes/WeatherMode.h"
#include "services/DisplayService.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <string>
#include <variant>

namespace
{
struct VectorStreamWriter final : public Stream
{
    std::vector<char> &target;
    explicit VectorStreamWriter(std::vector<char> &dest) : target(dest) {}
    size_t write(uint8_t byte) override
    {
        target.push_back(static_cast<char>(byte));
        return 1U;
    }
    size_t write(const uint8_t *buffer, size_t size) override
    {
        target.insert(
            target.end(), reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(buffer) + size);
        return size;
    }
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}
};
} // namespace

/**
 * @brief Fetch the configured weather URL over HTTPS using the embedded CA bundle.
 *
 * @param body Receives appended response bytes for HTTP 200; existing contents are preserved.
 * @param lastMillis Millisecond scheduling timestamp, adjusted for a 65,536 ms retry delay
 * on request initialization or transport failure; otherwise unchanged.
 * @return HTTP status, or -1 on initialization or transport failure. A 200 status
 * does not guarantee a complete body because stream write errors are not checked.
 */
int WeatherHandler::fetch(std::vector<char> &body, unsigned long &lastMillis)
{
    BearSSL::X509List trusted{CertificateHandler::bundle.data(), CertificateHandler::bundle.size()};
    BearSSL::WiFiClientSecure secure{};
    secure.setTrustAnchors(&trusted);
    HTTPClient http{};
    http.setUserAgent("Kortlinje/" VERSION " (RP2350; +https://github.com/VIPnytt/Kortlinje)");
    if (!http.begin(secure, std::string("https://").append(host).append(path).append("?").append(query).c_str()))
    {
        lastMillis = millis() - WeatherMode::interval + (0b1U << 16U);
        return -1;
    }
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");
    const int status{http.GET()};
    if (status <= 0)
    {
        http.end();
        lastMillis = millis() - WeatherMode::interval + (0b1U << 16U);
        return -1;
    }
    if (status != 200)
    {
        http.end();
        return status;
    }
    const int length{http.getSize()};
    if (length > 0)
    {
        body.reserve(static_cast<size_t>(length));
    }
    VectorStreamWriter writer(body);
    http.writeToStream(&writer);
    http.end();
    return status;
}

std::optional<WeatherHandler::Condition> WeatherHandler::getCondition(
    std::string_view code,
    std::span<const std::pair<WeatherHandler::Condition, std::span<const std::string_view>>> codesets)
{
    for (const auto &codeset : codesets)
    {
        if (std::find(codeset.second.begin(), codeset.second.end(), code) != codeset.second.end())
        {
            return codeset.first;
        }
    }
    return std::nullopt;
}

template <typename T>
    requires std::is_unsigned_v<T>
std::optional<WeatherHandler::Condition>
WeatherHandler::getCondition(T code, std::span<const std::pair<WeatherHandler::Condition, std::span<const T>>> codesets)
{
    for (const auto &codeset : codesets)
    {
        if (std::find(codeset.second.begin(), codeset.second.end(), code) != codeset.second.end())
        {
            return codeset.first;
        }
    }
    return std::nullopt;
}

template std::optional<WeatherHandler::Condition> WeatherHandler::getCondition<uint8_t>(
    uint8_t code, std::span<const std::pair<WeatherHandler::Condition, std::span<const uint8_t>>> codesets);
template std::optional<WeatherHandler::Condition> WeatherHandler::getCondition<uint16_t>(
    uint16_t code, std::span<const std::pair<WeatherHandler::Condition, std::span<const uint16_t>>> codesets);
template std::optional<WeatherHandler::Condition> WeatherHandler::getCondition<uint32_t>(
    uint32_t code, std::span<const std::pair<WeatherHandler::Condition, std::span<const uint32_t>>> codesets);
std::variant<std::span<const uint8_t>, std::span<const uint16_t>> WeatherHandler::getSign(Condition condition)
{
    switch (condition)
    {
    case Condition::CLEAR:
        return bitmapClear;
    case Condition::CLOUDY:
        return bitmapCloudy;
    case Condition::CLOUDY_PARTLY:
        return bitmapCloudyPartly;
    case Condition::FOG:
        return bitmapFog;
    case Condition::RAIN:
        return bitmapRain;
    case Condition::SNOW:
        return bitmapSnow;
    case Condition::THUNDER:
        return bitmapThunder;
    case Condition::WIND:
        return bitmapWind;
    }
    return {};
}
