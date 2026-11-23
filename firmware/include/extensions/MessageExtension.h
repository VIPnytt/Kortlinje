#pragma once

#include "fonts/SmallFont.h"
#include "handlers/TextHandler.h"
#include "modules/ExtensionModule.h"
#include "services/DisplayService.h"

#include <array>
#include <cstdint>
#include <optional>
#include <pico/time.h>
#include <string>
#include <string_view>

class MessageExtension final : public ExtensionModule
{
public:
    static constexpr std::string_view name{"Message"};

    explicit MessageExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void onHomeAssistant(JsonDocument &discovery, std::string_view unique) override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;

    static void setFont(std::string_view _fontName);
    static void setMessage(std::string_view _message);
    static void setRepeat(uint8_t _repeat);

private:
    bool active{false};
    static inline volatile bool pending{false};
    static inline volatile bool reload{false};
    volatile bool tick{false};

    uint8_t count{0U};
    static inline uint8_t repeat{3U};

    uint16_t step{0U};

    repeating_timer timer{};

    static inline std::string fontName{SmallFont::name};
    static inline std::string message{};

    std::array<bool, Display::grid::pixels> frame{};

    std::optional<TextHandler> text{};

    void stop();
    void transmit();

    [[nodiscard]] static bool onTimer(repeating_timer *rt);
};
