#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <cmath>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class CircleMode final : public ModeModule
{
private:
    static constexpr float x{Display::grid::maxX / 2.0F};
    static constexpr float y{Display::grid::maxY / 2.0F};

    static inline const uint8_t maxRadius{static_cast<uint8_t>(ceilf(hypotf(x, y)))};

    bool lit{true};
    volatile bool tick{false};

    uint8_t radius{0U};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Circle"};

    explicit CircleMode() : ModeModule(name) {};
    ~CircleMode() override;

    void begin() override;
    void handle() override;
};
