#pragma once

#include "modes/EqualizerMode.h"
#include "modes/SnakeMode.h"
#include "modes/TickerMode.h"
#include "modules/ModeModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <memory>

class ModesService final : public ServiceModule
{
private:
    explicit ModesService() : ServiceModule("Modes") {};

    unsigned long lastMillis{0UL};

    std::unique_ptr<ModeModule> mode{};

public:
    static constexpr auto names{std::to_array<std::string_view>({
        EqualizerMode::name,
        SnakeMode::name,
        TickerMode::name,
    })};

    void begin();
    void handle();
    [[nodiscard]] ModeModule *getMode();
    [[nodiscard]] std::unique_ptr<ModeModule> getMode(std::string_view modeName);
    void setMode(std::string_view modeName, bool power = true);
    void setModeNext();
    void setModePrevious();

    static ModesService &getInstance();
};

extern ModesService &Modes;
