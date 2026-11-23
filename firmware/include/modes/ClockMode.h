#pragma once

#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <string_view>
#include <time.h>

class ClockMode final : public ModeModule
{
private:
    volatile bool tick{false};

    int hour{24};
    int minute{60};

    repeating_timer timer{};

    time_t now{};

    tm local{};

    [[nodiscard]] static bool onTimer(repeating_timer *timer_data);

public:
    static constexpr std::string_view name{"Clock"};

    explicit ClockMode() : ModeModule(name) {};
    ~ClockMode() override;

    void begin() override;
    void handle() override;
};
