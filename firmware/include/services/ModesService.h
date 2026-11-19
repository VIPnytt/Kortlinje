#pragma once

#include "modules/ModeModule.h"
#include "modules/ServiceModule.h"

#include <cstdint>
#include <memory>
#include <pico/time.h>
#include <span>
#include <string>
#include <string_view>

class ModesService final : public ServiceModule
{

public:
    static constexpr std::string_view name{"Modes"};

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

    [[nodiscard]] static std::span<const std::string_view> getNames();

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
