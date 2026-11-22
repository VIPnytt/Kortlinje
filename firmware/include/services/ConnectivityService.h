#pragma once

#include "modules/ServiceModule.h"

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFiMulti.h>
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

    static void onHomeAssistant(JsonDocument &discovery, std::string_view unique);
    static void onReceive(JsonObjectConst payload, std::string_view source);

private:
    bool dnsRunning{false};
    bool mdns{false};
    static inline bool pendingConnect{false};
    static inline bool pendingScan{false};
    bool routable{false};
    bool scanning{false};
    static inline volatile bool tick{false};

    alarm_id_t alarmId{0L};

    static inline std::string pendingSsid{};
    static inline std::string pendingKey{};

    DNSServer dns{};

    WiFiMulti multi{};

    void connect(std::string_view ssid, std::string_view key);
    void initStation();
    void initHotspot();
    void onDisconnected();
    void onRoutable();
    void onScan(int8_t count);
    void schedule(uint64_t delay);
    void transmit();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);
};
