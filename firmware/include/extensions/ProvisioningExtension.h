#pragma once

#include "modules/ExtensionModule.h"

#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class ProvisioningExtension final : public ExtensionModule
{
public:
    explicit ProvisioningExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;

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

    /** Read-only access for portal HTML builder. */
    [[nodiscard]] static const std::vector<std::string> &scannedNetworksPublic() { return scannedNetworks; }

    /** Wipe LittleFS Wifi ssid/key. Does not reboot by itself. */
    static bool clearCredentials();

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
    static inline bool enterRequested{false};
    static inline unsigned long enteredMs{0UL};
    static inline unsigned long statusMs{0UL};
    static inline std::array<char, 6U> apName{}; // "K" + 4 hex + NUL
    static inline std::string pendingSsid{};
    static inline std::string pendingKey{};
    static inline WiFiUDP dnsUdp{};
    static inline bool dnsRunning{false};
    static inline bool ntpPending{false};
    static inline bool ntpStarted{false};
    static inline bool rebootPending{false};
    /** Creds exist but boot join failed — keep retrying STA, never auto-SoftAP. */
    static inline bool staRetryPending{false};
    static inline uint8_t joinAttempts{0U};
    static inline std::vector<std::string> scannedNetworks{};

    static constexpr uint8_t maxJoinAttempts{3U};
    static constexpr uint8_t bootJoinAttempts{3U};
    static constexpr unsigned long joinAttemptMs{12000UL};
    /** Short post-associate wait; finish DHCP in background so lwIP keeps running. */
    static constexpr unsigned long bootJoinWaitMs{3000UL};
    static constexpr unsigned long staRetryMs{1000UL};
    static constexpr unsigned long rebootDelayMs{800UL};

    static void startAp();
    static void stopAp();
    static void connectStation(std::string_view ssid, std::string_view key);
    static void finishOk();
    static void finishFail();
    static void buildApName();
    static void registerRoutes();
    static void startDns();
    static void stopDns();
    static void handleDns();
    static void requestNtp();
    static void pollNtp();
    /** STA scan before SoftAP — CYW43 cannot scan reliably while AP is up. */
    static void scanAndCache();
    /**
     * True when STA has an IPv4 address.
     * Do not use WiFi.connected() alone — arduino-pico only treats CYW43_LINK_JOIN as
     * connected, but after DHCP the driver often reports CYW43_LINK_UP.
     */
    [[nodiscard]] static bool stationOnline();
    /** Ensure STA netif link/up + DHCP renew (CYW43 often joins before DHCP moves). */
    static void kickStaDhcp();

    [[nodiscard]] static bool loadCredentials(std::string &ssid, std::string &key);
    [[nodiscard]] static bool saveCredentials(std::string_view ssid, std::string_view key);
    [[nodiscard]] static bool secretsFallback(std::string &ssid, std::string &key);

    void transmit();

    static void onGetPortal(AsyncWebServerRequest *request);
    static void onGetScan(AsyncWebServerRequest *request);
    static void onPostConnect(AsyncWebServerRequest *request);
    /** Captive probes: serve portal HTML directly (no redirect / no typing). */
    static void servePortal(AsyncWebServerRequest *request);
};
