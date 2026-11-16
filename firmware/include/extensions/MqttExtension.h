#pragma once

#include "constants/secrets.h"
#include "modules/ExtensionModule.h"

#include <Arduino.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include <format>
#include <string>
#include <string_view>

class MqttExtension final : public ExtensionModule
{
private:
    static constexpr size_t prefixLength{std::string_view{"kortlinje/"}.size() + Kortlinje::device::hostname.size() +
                                         std::string_view{"/"}.size()};
    static constexpr size_t suffixLength{std::string_view{"/set"}.size()};

    static constexpr std::string_view name{"MQTT"};

    unsigned long lastMillis{0UL};

    static inline const std::string discoveryTopic{std::format("homeassistant/device/0x{}/config", rp2040.getChipID())};

    static inline WiFiClient net{};

    static void onConnect();
    static void onMessage(MQTTClient *_client, char *topic, char *bytes, int length);

public:
    explicit MqttExtension() : ExtensionModule(name) {};

    static inline MQTTClient client{static_cast<int>(0b1U << 8U), static_cast<int>(0b1U << 10U)};

    void begin() override;
    void handle() override;

    void disconnect();

    void onTransmit(JsonObjectConst payload, std::string_view source) override;
};
