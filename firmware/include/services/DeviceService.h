#pragma once

#include "modules/ServiceModule.h"
#include "services/ConnectivityService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"
#include "services/ModesService.h"
#include "services/WebServerService.h"

#include <ArduinoJson.h>
#include <RP2040Support.h>
#include <atomic>
#include <string>

class DeviceService final : public ServiceModule
{
public:
    static constexpr std::string_view name{"Device"};

    static constexpr std::array<RP2040::resetReason_t, 3U> resetAbnormalities{
        RP2040::resetReason_t::BROWNOUT_RESET,
        RP2040::resetReason_t::GLITCH_RESET,
        RP2040::resetReason_t::WDT_RESET,
    };

    explicit DeviceService() : ServiceModule(name) {};

    void begin0();
    void begin1();
    void handle0();
    void handle1();

    static void onHomeAssistant(JsonDocument &discovery, std::string_view unique);
    static void receive(JsonObjectConst payload, std::string_view source, std::string_view destination);
    static void transmit(JsonObjectConst payload, std::string_view source, bool retain = true);

    [[nodiscard]] static JsonObjectConst getTransmits();

private:
    static constexpr uint8_t samples{(0b1U << 16U) / (0b1U << ADC_RESOLUTION)};

    float temperature{.0F};

    float voltage{.0F};

    std::atomic<bool> alive{false};

    std::string latest{};

    ConnectivityService connectivity{};

    DisplayService display{};

    ExtensionsService extensions{};

    FontsService fonts{};

    static inline JsonDocument transmits{};

    ModesService modes{};

    WebServerService webServer{};

    void release();
    void transmit();

    [[nodiscard]] uint16_t getSamples();
};
