#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <Arduino.h>
#include <bits/unique_ptr.h>
#include <deque>

class PingPongMode final : public ModeModule
{
private:
    static constexpr float speed{7e-6F * GRID_COLUMNS};

    float xDec{.0F};
    float yDec{.0F};

    std::deque<uint8_t> paddleL{};
    std::deque<uint8_t> paddleR{};

    uint8_t x{GRID_COLUMNS - 2U};
    uint8_t y{GRID_ROWS / 2U};

    uint16_t deg{135U};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Ping-pong"};

    explicit PingPongMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
};
