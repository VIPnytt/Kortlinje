#pragma once

#include "modules/ExtensionModule.h"

#include <AsyncUDP.h>
#include <ESPAsyncWebServer.h>
#include <array>
#include <cstdint>

class AlexaExtension final : public ExtensionModule
{
private:
    static inline AsyncUDP upnp{};

    static void onGetDescription(AsyncWebServerRequest *request);
    static void onGetLight(AsyncWebServerRequest *request);
    static void onGetLights(AsyncWebServerRequest *request);
    static void onPostApi(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index, size_t total);
    static void onPutState(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index, size_t total);
    static void onUpnp(AsyncUDPPacket &packet);

    [[nodiscard]] static uint64_t mac();

    [[nodiscard]] static std::array<char, 0b1U << 8U> light();

public:
    static constexpr std::string_view name{"Alexa"};

    explicit AlexaExtension() : ExtensionModule(name) {};

    void begin() override;

    static void onMdns();
};
