#pragma once

#include "modules/ServiceModule.h"

#include <ArduinoJson.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <pico/time.h>
#include <string>
#include <string_view>

class ConnectivityService final : public ServiceModule
{
public:
    static constexpr std::string_view name{"Connectivity"};

    explicit ConnectivityService() : ServiceModule(name) {};

    void begin();
    void handle();

    static void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique);
    static void onReceive(JsonObjectConst payload, std::string_view source);

private:
    bool dnsRunning{false};
    bool mdns{false};
    bool routable{false};
    bool scanning{false};

    repeating_timer timer{};

    WiFiMulti multi{};

    WiFiUDP dnsUdp{};

    static inline volatile bool pending{false};
    static inline bool pendingConnect{false};
    static inline bool pendingScan{false};

    static inline std::string pendingSsid{};
    static inline std::string pendingKey{};

    void connect(std::string_view ssid, std::string_view key);
    void handleDns();
    void initStation();
    void initHotspot();
    void onDisconnected();
    void onRoutable();
    void onScan(int8_t count);
    void transmit();

    static bool onTimer(repeating_timer *timer);
};
