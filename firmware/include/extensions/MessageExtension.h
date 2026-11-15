#pragma once

#include "config/IkeaKortlinje.h"
#include "fonts/SmallFont.h"
#include "handlers/TextHandler.h"
#include "modules/ExtensionModule.h"
#include "modules/FontModule.h"

#include <array>
#include <cstdint>
#include <memory>
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
    void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique) override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;

    static void setFont(std::string_view _fontName);
    static void setMessage(std::string_view _message);
    static void setRepeat(uint8_t _repeat);

private:
    static inline bool active{false};
    static inline volatile bool pending{false};
    static inline volatile bool reload{false};
    volatile bool tick{false};
    bool timerRunning{false};

    static inline uint8_t count{0U};
    static inline uint8_t offsetY{0U};
    static inline uint8_t repeat{3U};
    static inline uint8_t width{0U};

    static inline int16_t offsetX{GRID_COLUMNS};

    static inline repeating_timer timer{};

    static inline std::string fontName{SmallFont::name};
    static inline std::string message{};

    static inline std::array<bool, GRID_COLUMNS * GRID_ROWS> frame{};

    static inline std::unique_ptr<const FontModule> font{};
    static inline std::unique_ptr<TextHandler> text{};

    static void transmit();

    [[nodiscard]] static bool onTimer(repeating_timer *rt);
};