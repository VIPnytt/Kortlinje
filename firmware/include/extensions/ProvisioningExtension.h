#pragma once

#include "modules/ExtensionModule.h"

#include <ESPAsyncWebServer.h>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

class ProvisioningExtension final : public ExtensionModule
{
public:
    explicit ProvisioningExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;

    static void enter();
    static void exit();
    static void toggle();

    [[nodiscard]] static bool isActive();

    /** Draw AP name / status. Call from display core only. */
    static void draw();

    /** Try LittleFS WiFi then secrets.h; begin STA. false → caller should SoftAP. */
    [[nodiscard]] static bool beginStation();

    /** Captive / unknown paths while SoftAP is up. */
    [[nodiscard]] static bool handleNotFound(AsyncWebServerRequest *request);

private:
    static constexpr std::string_view name{"Provisioning"};

    static constexpr std::string_view configModule{"Wifi"};
    static constexpr unsigned long timeoutMs{5UL * 60UL * 1000UL};
    static constexpr uint8_t apChannel{1U};

    enum class State : uint8_t
    {
        Idle,
        ApWait,
        Joining,
        Ok,
        Fail,
    };

    static inline State state{State::Idle};
    static inline unsigned long enteredMs{0UL};
    static inline unsigned long statusMs{0UL};
    static inline std::array<char, 6U> apName{}; // "K" + 4 hex + NUL
    static inline std::string pendingSsid{};
    static inline std::string pendingKey{};

    static void startAp();
    static void stopAp();
    static void connectStation(std::string_view ssid, std::string_view key);
    static void finishOk();
    static void finishFail();
    static void buildApName();
    static void registerRoutes();

    [[nodiscard]] static bool loadCredentials(std::string &ssid, std::string &key);
    [[nodiscard]] static bool saveCredentials(std::string_view ssid, std::string_view key);
    [[nodiscard]] static bool secretsFallback(std::string &ssid, std::string &key);

    static void onGetPortal(AsyncWebServerRequest *request);
    static void onGetScan(AsyncWebServerRequest *request);
    static void onPostConnect(AsyncWebServerRequest *request);
    static void redirectPortal(AsyncWebServerRequest *request);
};
