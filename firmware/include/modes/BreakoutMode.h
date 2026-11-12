#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <cstdint>
#include <ctime>
#include <pico/time.h>
#include <string_view>

class BreakoutMode final : public ModeModule
{
private:
    bool pending{false};
    volatile bool tick{false};

    uint8_t paddleY{GRID_ROWS / 2U};
    uint8_t x{GRID_COLUMNS - 2U};
    uint8_t y{GRID_ROWS / 2U};

    uint16_t deg{180U};

    int hour{24};
    int minute{60};

    float xDec{static_cast<float>(GRID_COLUMNS - 2U)};
    float yDec{static_cast<float>(GRID_ROWS / 2U)};

    alarm_id_t alarmId{0L};

    time_t now{};

    tm local{};

    void startAlarm();
    void stopAlarm();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Breakout"};

    explicit BreakoutMode() : ModeModule(name) {};
    ~BreakoutMode() override;

    void begin() override;
    void handle() override;
    void end() override;
};
