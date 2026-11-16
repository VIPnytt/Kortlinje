#pragma once

#include "modules/ExtensionModule.h"
#include "services/DisplayService.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class SignExtension final : public ExtensionModule
{
private:
    bool enabled{false};
    bool pending{false};
    volatile bool tick{false};

    uint32_t duration{0b1U << 15U};

    volatile alarm_id_t alarmId{0L};

    std::array<bool, Display::grid::pixels> frame{};

    std::array<uint64_t, Display::grid::rows> bitmap{};

    void setDuration(uint8_t seconds);
    void transmit();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Sign"};

    explicit SignExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
