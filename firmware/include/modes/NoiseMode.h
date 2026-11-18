#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class NoiseMode final : public ModeModule
{
private:
    volatile bool tick{false};

    std::array<size_t, Display::grid::pixels / (0b1U << 3U)> dots{};
    std::array<size_t, Display::grid::pixels / (0b1U << 3U)>::iterator it{dots.begin()};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Noise"};

    explicit NoiseMode() : ModeModule(name) {};
    ~NoiseMode() override;

    void begin() override;
    void handle() override;
};
