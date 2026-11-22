#pragma once

#include "constants/secrets.h"
#include "handlers/TextHandler.h"
#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <bits/unique_ptr.h>
#include <memory>
#include <pico/time.h>
#include <string>
#include <string_view>

class TickerMode final : public ModeModule
{
private:
    static inline bool pending{false};
    bool timerRunning{false};
    volatile bool tick{false};

    int8_t offsetY{static_cast<int8_t>(Display::grid::rows / 2U)};

    int16_t offsetX{Display::grid::columns};
    int16_t width{0};

    repeating_timer timer{};

    static inline std::string message{Kortlinje::device::name};

    std::unique_ptr<const FontModule> font{};

    std::unique_ptr<TextHandler> text{};

    void setFont(std::string_view fontName);
    void setMessage(std::string_view _message);
    void startTimer();
    void stopTimer();
    void transmit();

    [[nodiscard]] static bool onTimer(repeating_timer *rt);

public:
    static constexpr std::string_view name{"Ticker"};

    explicit TickerMode() : ModeModule(name) {};
    ~TickerMode() override;

    void begin() override;
    void handle() override;
    void onHomeAssistant(JsonDocument &discovery, std::string_view unique) override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
