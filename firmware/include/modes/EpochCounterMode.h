#pragma once

#include "modules/ModeModule.h"

class EpochCounterMode final : public ModeModule
{
private:
    time_t epoch{0LL};

public:
    static constexpr std::string_view name{"Epoch counter"};

    explicit EpochCounterMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};
