#pragma once

#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <string_view>
#include <time.h>

class WeekdayMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 7U> weekdays{
        "SUN",
        "MON",
        "TUE",
        "WED",
        "THU",
        "FRI",
        "SAT",
    };

    volatile bool tick{false};

    int weekday{7};

    repeating_timer timer{};

    time_t now{};

    tm local{};

    [[nodiscard]] static bool onTimer(repeating_timer *timer_data);

public:
    static constexpr std::string_view name{"Weekday"};

    explicit WeekdayMode() : ModeModule(name) {};
    ~WeekdayMode() override;

    void begin() override;
    void handle() override;
};
