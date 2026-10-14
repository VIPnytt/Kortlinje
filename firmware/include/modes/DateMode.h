#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <array>
#include <string_view>
#include <time.h>

class DateMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 12U> names{
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

    bool pending{false};

    int day{0};
    int month{12};

    time_t now{};

    tm local{};

public:
    static constexpr std::string_view name{"Date"};

    explicit DateMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};
