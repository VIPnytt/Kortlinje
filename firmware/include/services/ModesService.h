#pragma once

#include "modes/ClockMode.h"       //NOLINT(misc-include-cleaner)
#include "modes/DateMode.h"        //NOLINT(misc-include-cleaner)
#include "modes/EqualizerMode.h"   //NOLINT(misc-include-cleaner)
#include "modes/HumidityMode.h"    //NOLINT(misc-include-cleaner)
#include "modes/PingPongMode.h"    //NOLINT(misc-include-cleaner)
#include "modes/SnakeMode.h"       //NOLINT(misc-include-cleaner)
#include "modes/TemperatureMode.h" //NOLINT(misc-include-cleaner)
#include "modes/TickerMode.h"      //NOLINT(misc-include-cleaner)
#include "modes/WeatherMode.h"     //NOLINT(misc-include-cleaner)
#include "modes/WeekdayMode.h"     //NOLINT(misc-include-cleaner)
#include "modules/ModeModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <memory>

class ModesService final : public ServiceModule
{

public:
    static constexpr std::string_view name{"Modes"};

    static constexpr std::array<std::string_view, 10U> names{
        ClockMode::name,
        DateMode::name,
        EqualizerMode::name,
        HumidityMode::name,
        PingPongMode::name,
        SnakeMode::name,
        TickerMode::name,
        TemperatureMode::name,
        WeatherMode::name,
        WeekdayMode::name,
    };

    explicit ModesService() : ServiceModule(name) {};

    void begin();
    void handle();
    void setModeNext();
    void setModePrevious();

    static void onReceive(JsonObjectConst payload, std::string_view source);
    static void setMode(std::string_view modeName, bool power = true);

    [[nodiscard]] static ModeModule *getMode();

    [[nodiscard]] static std::unique_ptr<ModeModule> getMode(std::string_view modeName);

private:
    unsigned long lastMillis{0UL};

    static inline std::unique_ptr<ModeModule> mode{};

    static void transmit();
};
