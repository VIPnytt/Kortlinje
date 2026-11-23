#pragma once

#include "constants/secrets.h"
#include "handlers/TextHandler.h"
#include "modules/FontModule.h"
#include "modules/ModeModule.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <pico/time.h>
#include <string>
#include <string_view>

class TickerMode final : public ModeModule
{
private:
    volatile bool pending{false};
    volatile bool tick{false};

    uint16_t step{0U};

    repeating_timer timer{};

    static inline std::string message{Kortlinje::device::name};

    std::unique_ptr<const FontModule> font{};
    std::optional<TextHandler> text{};

    void setFont(std::string_view fontName);
    void setMessage(std::string_view _message);
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
