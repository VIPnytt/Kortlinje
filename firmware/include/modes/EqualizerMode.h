#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <ArduinoOTA.h>
#include <array>

class EqualizerMode final : public ModeModule
{
private:
    static constexpr uint8_t width{3U};

    std::array<std::pair<uint8_t, uint8_t>, GRID_COLUMNS / (width + 1U)> bars{};

    unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Equalizer"};

    explicit EqualizerMode() : ModeModule(name) {};

    void begin();
    void handle();
};
