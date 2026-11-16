#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <cstdint>
#include <ctime>
#include <pico/time.h>
#include <string_view>

class BreakoutMode final : public ModeModule
{
private:
    volatile bool tick{false};

    uint8_t paddleY{Display::grid::rows / 2U};
    uint8_t x{Display::grid::maxX - 1U};
    uint8_t y{Display::grid::rows / 2U};

    uint16_t deg{180U};

    int hour{24};
    int minute{60};

    float xDec{static_cast<float>(Display::grid::maxX - 1U)};
    float yDec{static_cast<float>(Display::grid::rows / 2U)};

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
