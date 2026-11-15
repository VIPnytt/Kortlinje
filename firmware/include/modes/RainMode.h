#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class RainMode final : public ModeModule
{
private:
    struct Drop
    {
        uint8_t x{0U};
        uint8_t y{GRID_ROWS};
        uint8_t length{0U};
        uint8_t delay{1U};
        uint8_t ticks{0U};
    };

    volatile bool tick{false};

    repeating_timer timer{};

    std::array<Drop, GRID_COLUMNS / 4U> drops{};

    [[nodiscard]]  static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Rain"};

    explicit RainMode() : ModeModule(name) {};
    ~RainMode() override;

    void begin() override;
    void handle() override;
};
