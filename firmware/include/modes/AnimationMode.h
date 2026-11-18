#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <array>
#include <cstdint>
#include <pico/time.h>
#include <span>
#include <string_view>
#include <vector>

class AnimationMode final : public ModeModule
{
private:
    bool pending{true};
    volatile bool tick{false};
    bool timerRunning{false};

    uint8_t index{0U};

    uint32_t interval{0b1U << 19U};

    repeating_timer timer{};

    std::vector<std::array<uint8_t, Display::grid::bytes>> frames{};

    void setFrame(uint8_t _index, std::span<const uint8_t> frame);
    void setFrames(uint8_t count);
    void setInterval(uint32_t milliseconds);

    void startTimer();
    void stopTimer();

    void transmit(uint8_t _index, std::span<const uint8_t> frame);

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Animation"};

    explicit AnimationMode() : ModeModule(name) {};
    ~AnimationMode() override;

    void begin() override;
    void handle() override;

    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
