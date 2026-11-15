#pragma once

#include "modules/ModeModule.h"

class BrightMode final : public ModeModule
{
public:
    static constexpr std::string_view name{"Bright"};

    explicit BrightMode() : ModeModule(name) {};

    void begin() override;
};
