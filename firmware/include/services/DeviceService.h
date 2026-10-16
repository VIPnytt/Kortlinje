#pragma once

#include "modules/ServiceModule.h"

#include <Arduino.h>
#include <ArduinoJson.h> // NOLINT(misc-include-cleaner)

class DeviceService final : public ServiceModule
{

public:
    static constexpr std::string_view name{"Device"};

    void begin0();
    void begin1();
    void handle0();
    void handle1();
    void receive(JsonObjectConst payload, std::string_view source, std::string_view destination) const;
    void transmit(JsonObjectConst payload, std::string_view source, bool retain = true);

    [[nodiscard]] JsonObjectConst getTransmits() const;

    static DeviceService &getInstance();

private:
    explicit DeviceService() : ServiceModule(name) {};

    bool operational{false};

    float temperature{.0F};

    uint16_t voltage{0U};

    unsigned long lastMillis{0UL};

    JsonDocument transmits{};

    void onReceive(JsonObjectConst payload, std::string_view source) override;
    void transmit();
};

extern DeviceService &Device;
