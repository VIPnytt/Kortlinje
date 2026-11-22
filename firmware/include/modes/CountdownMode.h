#pragma once

#include "modules/ModeModule.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

class CountdownMode final : public ModeModule
{
private:
    static inline std::chrono::time_point<std::chrono::system_clock> epoch{};

    bool odd{false};

    uint8_t blink{0U};
    uint8_t minutes{0U};
    uint8_t seconds{0U};

    void transmit();

public:
    static constexpr std::string_view name{"Countdown"};

    explicit CountdownMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    void onHomeAssistant(JsonDocument &discovery, std::string_view unique) override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
