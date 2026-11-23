#pragma once

#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <string_view>
#include <time.h>

class DateMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 12U> months{
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC",
    };

    volatile bool tick{false};

    int day{0};
    int month{12};

    repeating_timer timer{};

    time_t now{};

    tm local{};

    [[nodiscard]] static bool onTimer(repeating_timer *timer_data);

public:
    static constexpr std::string_view name{"Date"};

    explicit DateMode() : ModeModule(name) {};
    ~DateMode() override;

    void begin() override;
    void handle() override;
};
