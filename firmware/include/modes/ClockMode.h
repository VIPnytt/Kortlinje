#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <time.h>

class ClockMode final : public ModeModule
{
private:
    bool pending{false};

    int hour{24};
    int minute{60};

    time_t now{};

    tm local{};

public:
    static constexpr std::string_view name{"Clock"};

    explicit ClockMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};
