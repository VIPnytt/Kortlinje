#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <array>
#include <string_view>
#include <time.h>

class WeekdayMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 7U> names{
        "SUN",
        "MON",
        "TUE",
        "WED",
        "THU",
        "FRI",
        "SAT",
    };

    bool pending{false};

    int weekday{7};

    time_t now{};

    tm local{};

public:
    static constexpr std::string_view name{"Weekday"};

    explicit WeekdayMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};
