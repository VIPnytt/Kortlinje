#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <api/Common.h>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class WaveformMode final : public ModeModule
{
private:
    static constexpr float centerY{(GRID_ROWS - 1U) / 2.0F};
    static constexpr float ampDrift{centerY * .28F};
    static constexpr float baseAmp{centerY * .72F};
    static constexpr float baseFreq{.26F};
    static constexpr float freqDrift{.10F};
    static constexpr float twoPi{static_cast<float>(TWO_PI)};

    volatile bool tick{false};

    float ampPhase{.0F};
    float freqPhase{.0F};
    float wavePhase{.0F};

    repeating_timer timer{};

    void renderColumn(uint8_t x);

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

    [[nodiscard]] static float wrapPhase(float phase);

public:
    static constexpr std::string_view name{"Waveform"};

    explicit WaveformMode() : ModeModule(name) {};
    ~WaveformMode() override;

    void begin() override;
    void handle() override;
};
