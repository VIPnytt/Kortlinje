#pragma once

#include "config/IkeaKortlinje.h"
#include "handlers/TextHandler.h"
#include "modules/ModeModule.h"

#include <bits/unique_ptr.h>

class TickerMode final : public ModeModule
{
private:
    static inline std::string message{"Kortlinje"};

    bool pending{false};

    int8_t offsetY{static_cast<int8_t>(GRID_ROWS / 2U)};

    int16_t offsetX{GRID_COLUMNS};
    int16_t width{0};

    unsigned long lastMillis{0UL};

    std::unique_ptr<const FontModule> font{};

    std::unique_ptr<TextHandler> text{};

    void setFont(std::string_view fontName);
    void setMessage(std::string_view _message);

public:
    static constexpr std::string_view name{"Ticker"};

    explicit TickerMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
    void end() override;
};
