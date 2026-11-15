#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class LeafFallMode final : public ModeModule
{
private:
    struct Leaf
    {
        uint8_t x{0U};
        uint8_t y{0U};
    };

    volatile bool tick{false};

    std::array<Leaf, GRID_COLUMNS * GRID_ROWS / 20U> leaves{};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Leaf fall"};

    explicit LeafFallMode() : ModeModule(name) {};
    ~LeafFallMode() override;

    void begin() override;
    void handle() override;
};
