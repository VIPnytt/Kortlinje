#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <pico/time.h>

class PingPongMode final : public ModeModule
{
private:
    volatile bool tick{false};

    uint8_t paddleL{Display::grid::rows / 2U};
    uint8_t paddleR{Display::grid::rows / 2U};
    uint8_t x{Display::grid::maxX - 1U};
    uint8_t y{Display::grid::rows / 2U};

    uint16_t deg{180U};

    volatile int64_t interval{static_cast<int64_t>(0b1U << 15U)};

    float xDec{static_cast<float>(x)};
    float yDec{static_cast<float>(y)};

    alarm_id_t alarmId{0L};

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Ping-pong"};

    explicit PingPongMode() : ModeModule(name) {};
    ~PingPongMode() override;

    void begin() override;
    void handle() override;
};
