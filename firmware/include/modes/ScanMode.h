#pragma once

#include "modules/ModeModule.h"

#include <cstdint>
#include <pico/time.h>
#include <string_view>

class ScanMode final : public ModeModule
{
private:
    bool timerRunning{false};
    volatile bool tick{false};

    uint8_t x{0U};

    repeating_timer timer{};

    void startTimer();
    void stopTimer();

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Scan"};

    explicit ScanMode() : ModeModule(name) {};
    ~ScanMode() override;

    void begin() override;
    void handle() override;
    void end() override;
};
