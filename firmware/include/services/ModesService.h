#pragma once

#include "modes/ClockMode.h"       //NOLINT(misc-include-cleaner)
#include "modes/DateMode.h"        //NOLINT(misc-include-cleaner)
#include "modes/EqualizerMode.h"   //NOLINT(misc-include-cleaner)
#include "modes/HumidityMode.h"    //NOLINT(misc-include-cleaner)
#include "modes/PingPongMode.h"    //NOLINT(misc-include-cleaner)
#include "modes/SnakeMode.h"       //NOLINT(misc-include-cleaner)
#include "modes/TemperatureMode.h" //NOLINT(misc-include-cleaner)
#include "modes/TickerMode.h"      //NOLINT(misc-include-cleaner)
#include "modes/WeekdayMode.h"     //NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <memory>

class ModesService final : public ServiceModule
{

public:
    static constexpr std::string_view name{"Modes"};

    static constexpr std::array<std::string_view, 9U> names{
        ClockMode::name,
        DateMode::name,
        EqualizerMode::name,
        HumidityMode::name,
        PingPongMode::name,
        SnakeMode::name,
        TickerMode::name,
        TemperatureMode::name,
        WeekdayMode::name,
    };

    void begin();
    void handle();
    void onReceive(JsonObjectConst payload, std::string_view source) override;
    void setMode(std::string_view modeName, bool power = true);
    void setModeNext();
    void setModePrevious();

    [[nodiscard]] ModeModule *getMode();

    [[nodiscard]] std::unique_ptr<ModeModule> getMode(std::string_view modeName);

    static ModesService &getInstance();

private:
    explicit ModesService() : ServiceModule(name) {};

    unsigned long lastMillis{0UL};

    std::unique_ptr<ModeModule> mode{};

    void transmit();
};

extern ModesService &Modes;
