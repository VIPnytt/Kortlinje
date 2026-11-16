#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <ArduinoOTA.h>
#include <array>
#include <pico/time.h>

class EqualizerMode final : public ModeModule
{
private:
    static constexpr uint8_t width{3U};

    std::array<std::pair<uint8_t, uint8_t>, Display::grid::columns / (width + 1U)> bars{};

    volatile bool tick{false};

    repeating_timer timer{};

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Equalizer"};

    explicit EqualizerMode() : ModeModule(name) {};
    ~EqualizerMode() override;

    void begin() override;
    void handle() override;
};
