#pragma once

#include "modules/ServiceModule.h"

#include <ESPAsyncWebServer.h>
#include <array>
#include <span>

class WebServerService final : public ServiceModule
{
public:
    static constexpr std::string_view name{"Web server"};

    static consteval std::array<char, 11U> etag(std::span<const uint8_t> data)
    {
        uint32_t hash{2'166'136'261U};
        for (const uint8_t byte : data)
        {
            hash ^= byte;
            hash *= 16'777'619U;
        }
        std::array<char, 11U> out{{'"', '0', '0', '0', '0', '0', '0', '0', '0', '"', '\0'}};
        for (size_t idx{1U}; idx < 9U; ++idx)
        {
            const uint8_t nibble{static_cast<uint8_t>((hash >> ((8U - idx) * 4U)) & 0xFU)};
            out[idx] = static_cast<char>(nibble < 10U ? ('0' + nibble) : ('A' + (nibble - 10U)));
        }
        return out;
    }

    template <std::size_t N> [[nodiscard]] static consteval auto embed(const unsigned char (&&raw)[N])
    {
        return std::to_array(raw);
    }

    AsyncWebServer http{80U};

    void begin();

    static void onEmpty(AsyncWebServerRequest *request);

    static WebServerService &getInstance();

private:
    explicit WebServerService() : ServiceModule(name) {};

    static void onNotFound(AsyncWebServerRequest *request);
};

extern WebServerService &WebServer;
