#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <cstdint>
#include <pico/time.h>
#include <string_view>

class PingPongMode final : public ModeModule
{
private:
    volatile bool tick{false};

    int8_t stepX{-1};

    uint8_t paddleL{Display::grid::rows / 2U};
    uint8_t paddleR{Display::grid::rows / 2U};
    uint8_t x{Display::grid::maxX - 1U};
    uint8_t y{Display::grid::rows / 2U};

    volatile int64_t interval{static_cast<int64_t>(0b1U << 15U)};

    float stepY{.0F};
    float yDec{static_cast<float>(Display::grid::rows / 2U)};

    alarm_id_t alarmId{0L};

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Ping-pong"};

    explicit PingPongMode() : ModeModule(name) {};
    ~PingPongMode() override;

    void begin() override;
    void handle() override;
};
