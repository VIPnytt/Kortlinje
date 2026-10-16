#pragma once

#include "modules/ServiceModule.h"

#include <ESPAsyncWebServer.h>
#include <span>

class WebServerService final : public ServiceModule
{
public:
    static constexpr std::string_view name{"Web server"};

    static inline bool render{false};

    void begin();
    void handle();
    void onTransmit(JsonObjectConst payload, std::string_view source);

    static void onEmpty(AsyncWebServerRequest *request);

    static WebServerService &getInstance();

private:
    static constexpr uint8_t INDEX_HTML[] = {
#embed "../../../webui/dist/index.html.gz"
    };

    static constexpr std::span<const uint8_t> INDEX_HTML_VIEW{INDEX_HTML};

    static constexpr size_t length{sizeof(INDEX_HTML)};

    static constexpr uint32_t calculateHash(std::span<const uint8_t> data)
    {
        uint32_t hash{2'166'136'261U};
        for (const uint8_t byte : data)
        {
            hash ^= byte;
            hash *= 16'777'619U;
        }
        return hash;
    }

    AsyncWebServer http{80U};

    AsyncWebSocket *socket{new AsyncWebSocket("/ws")};

    static inline std::array<char, 11U> etag{"\"00000000\""};

    static inline std::vector<uint8_t> buffer{};

    explicit WebServerService() : ServiceModule(name) {};

    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg,
                        const uint8_t *data, size_t len);
    static void onGetRoot(AsyncWebServerRequest *request);
    static void onHeadRoot(AsyncWebServerRequest *request);
    static void onNotFound(AsyncWebServerRequest *request);
};

extern WebServerService &WebServer;
