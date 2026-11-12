#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <pico/time.h>

class PingPongMode final : public ModeModule
{
private:
    volatile bool tick{false};

    uint8_t paddleL{GRID_ROWS / 2U};
    uint8_t paddleR{GRID_ROWS / 2U};
    uint8_t x{GRID_COLUMNS - 2U};
    uint8_t y{GRID_ROWS / 2U};

    uint16_t deg{135U};

    float xDec{.0F};
    float yDec{.0F};

    alarm_id_t alarmId{0L};

    void startAlarm();
    void stopAlarm();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Ping-pong"};

    explicit PingPongMode() : ModeModule(name) {};
    ~PingPongMode() override;

    void begin() override;
    void handle() override;
    void end() override;
};
