#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <cstdint>
#include <pico/time.h>
#include <string_view>

class BlindsMode final : public ModeModule
{
private:
    static constexpr uint8_t moduloMax{Display::grid::columns / 3U};

    bool direction{false};
    volatile bool tick{false};

    uint8_t modulo{moduloMax};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Blinds"};

    explicit BlindsMode() : ModeModule(name) {};
    ~BlindsMode() override;

    void begin() override;
    void handle() override;
};
