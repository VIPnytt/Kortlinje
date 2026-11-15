#pragma once

#include "modules/ModeModule.h"

#include <cstdint>
#include <pico/time.h>
#include <string_view>

class LinesMode final : public ModeModule
{
private:
    volatile bool tick{false};

    uint8_t x{0U};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Lines"};

    explicit LinesMode() : ModeModule(name) {};
    ~LinesMode() override;

    void begin() override;
    void handle() override;
};
