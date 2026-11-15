#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class StarsMode final : public ModeModule
{
private:
    struct Star
    {
        bool lit{false};
        uint8_t delay{0U};
        size_t idx{0U};
    };

    volatile bool tick{false};

    repeating_timer timer{};

    std::array<Star, GRID_COLUMNS * GRID_ROWS / (0b1U << 4U)> stars{};

   [[nodiscard]]   static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Stars"};

    explicit StarsMode() : ModeModule(name) {};
    ~StarsMode() override;

    void begin() override;
    void handle() override;
};
