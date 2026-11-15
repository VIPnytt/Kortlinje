#pragma once

#include "config/IkeaKortlinje.h" // NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <string_view>

class DrawMode final : public ModeModule
{
private:
    bool pending{false};
    bool render{false};

    std::array<uint8_t, ((GRID_COLUMNS * GRID_ROWS) + 7U) / 8U> frame{};

    void load(bool cache = false);
    void save(bool cache = false);
    void transmit();

public:
    static constexpr std::string_view name{"Draw"};

    explicit DrawMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;
    void end() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
