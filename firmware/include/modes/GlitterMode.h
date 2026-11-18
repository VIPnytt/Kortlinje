#pragma once

#include "modules/ModeModule.h"

class GlitterMode final : public ModeModule
{
private:
    volatile bool tick{false};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Glitter"};

    explicit GlitterMode() : ModeModule(name) {};
    ~GlitterMode() override;

    void begin() override;
    void handle() override;
};
