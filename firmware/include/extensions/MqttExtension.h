#pragma once

#include "constants/Kortlinje.h"
#include "constants/secrets.h"
#include "modules/ExtensionModule.h"

#include <MQTTClient.h>
#include <WiFi.h>
#include <format>
#include <meta>
#include <pico/time.h>
#include <string>
#include <string_view>

class MqttExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view topicPrefix{
        std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/")};
    static constexpr std::string_view availabilityTopic{
        std::define_static_string(std::string{topicPrefix} + "availability")};
    static constexpr std::string_view commandTopic{std::define_static_string(std::string{topicPrefix} + "+/set")};

    static constexpr size_t suffixLength{std::string_view{"/set"}.size()};

    static inline bool discovered{false};
    static inline volatile bool tick{true};

    static inline const std::string discoveryTopic{std::format("homeassistant/device/0x{}/config", rp2040.getChipID())};

    static inline WiFiClient net{};

    static void discover();
    static void onMessage(MQTTClient *_client, char *topic, char *bytes, int length);

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"MQTT"};

    explicit MqttExtension() : ExtensionModule(name) {};

    static inline MQTTClient client{static_cast<int>(0b1U << 7U), static_cast<int>(0b1U << 8U)};

    void begin() override;
    void handle() override;
    void onTransmit(JsonObjectConst payload, std::string_view source) override;

    static void disconnect();
};
