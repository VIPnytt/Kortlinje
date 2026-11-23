#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <cstdint>
#include <ctime>
#include <optional>
#include <pico/time.h>
#include <string_view>

class BreakoutMode final : public ModeModule
{
private:
    struct Hit
    {
        bool flipX{};
        uint8_t x{};
        uint8_t y{};
    };

    volatile bool tick{false};

    int8_t stepX{-1};

    uint8_t paddleY{Display::grid::rows / 2U};
    uint8_t x{Display::grid::maxX - 1U};
    uint8_t y{Display::grid::rows / 2U};

    int hour{24};
    int minute{60};

    volatile int64_t interval{static_cast<int64_t>(0b1U << 16U)};

    float stepY{.0F};
    float yDec{static_cast<float>(Display::grid::rows / 2U)};

    alarm_id_t alarmId{0L};

    time_t now{};

    tm local{};

    void reflectWalls();

    [[nodiscard]] std::optional<Hit> findHit() const;

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Breakout"};

    explicit BreakoutMode() : ModeModule(name) {};
    ~BreakoutMode() override;

    void begin() override;
    void handle() override;
};
