#pragma once

#include "modules/ModeModule.h"

#include <pico/time.h>
#include <string_view>

class BlinkMode final : public ModeModule
{
private:
    bool lit{false};
    volatile bool tick{false};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Blink"};

    explicit BlinkMode() : ModeModule(name) {};
    ~BlinkMode() override;

    void begin() override;
    void handle() override;
};
