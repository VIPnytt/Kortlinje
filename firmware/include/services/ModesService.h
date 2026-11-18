#pragma once

#include "modes/AnimationMode.h"
#include "modes/BlindsMode.h"
#include "modes/BlinkMode.h"
#include "modes/BreakoutMode.h"
#include "modes/BrightMode.h"
#include "modes/CircleMode.h"
#include "modes/ClockMode.h"
#include "modes/CountdownMode.h"
#include "modes/DateMode.h"
#include "modes/DrawMode.h"
#include "modes/EpochCounterMode.h"
#include "modes/EqualizerMode.h"
#include "modes/FireworkMode.h"
#include "modes/GameOfLifeMode.h"
#include "modes/GlitterMode.h"
#include "modes/HumidityMode.h"
#include "modes/LeafFallMode.h"
#include "modes/LinesMode.h"
#include "modes/NoiseMode.h"
#include "modes/PingPongMode.h"
#include "modes/RainMode.h"
#include "modes/ScanMode.h"
#include "modes/SnakeMode.h"
#include "modes/StarsMode.h"
#include "modes/TemperatureMode.h"
#include "modes/TickerMode.h"
#include "modes/WaveformMode.h"
#include "modes/WeatherMode.h"
#include "modes/WeekdayMode.h"
#include "modules/ModeModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <cstdint>
#include <memory>
#include <pico/time.h>
#include <string>
#include <string_view>

class ModesService final : public ServiceModule
{

public:
    static constexpr std::string_view name{"Modes"};

    static constexpr std::array<std::string_view, 29U> names{
        AnimationMode::name,
        BlindsMode::name,
        BlinkMode::name,
        BreakoutMode::name,
        BrightMode::name,
        CircleMode::name,
        ClockMode::name,
        CountdownMode::name,
        DateMode::name,
        DrawMode::name,
        EpochCounterMode::name,
        EqualizerMode::name,
        FireworkMode::name,
        GameOfLifeMode::name,
        GlitterMode::name,
        HumidityMode::name,
        LeafFallMode::name,
        LinesMode::name,
        NoiseMode::name,
        PingPongMode::name,
        RainMode::name,
        ScanMode::name,
        SnakeMode::name,
        StarsMode::name,
        TemperatureMode::name,
        TickerMode::name,
        WaveformMode::name,
        WeatherMode::name,
        WeekdayMode::name,
    };

    explicit ModesService() : ServiceModule(name) {};

    void begin();
    void handle();

    static void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique);
    static void onReceive(JsonObjectConst payload, std::string_view source);
    static void setActive(bool _active);
    static void setMode(std::string_view modeName, bool power = true, bool splash = true);
    static void setModeNext();
    static void setModePrevious();

    [[nodiscard]] static bool getActive();

    [[nodiscard]] static ModeModule *getMode();

    [[nodiscard]] static std::unique_ptr<ModeModule> getMode(std::string_view modeName);

private:
    static inline volatile bool active{true};
    static inline volatile bool busy{false};
    static inline volatile bool pending{false};
    static inline bool scheduled{false};
    static inline volatile bool tick{false};

    static inline alarm_id_t alarmId{0L};

    static inline std::unique_ptr<ModeModule> mode{};

    static void cancelAlarm();
    static void transmit();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);
};
