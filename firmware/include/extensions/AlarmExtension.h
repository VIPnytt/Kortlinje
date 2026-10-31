#pragma once

#include "modules/ExtensionModule.h"

#include <hardware/pio.h>
#include <pico/time.h>
#include <string_view>
#include <time.h>

class AlarmExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Alarm"};

    enum class State : uint8_t
    {
        IDLE,
        RINGING,
        SNOOZED
    };

    struct AlarmTime
    {
        bool enabled{false};
        uint8_t hour{0U};
        uint8_t minute{0U};
    };

    static inline bool pending{true};
    static inline volatile bool timeout{false};

    static inline volatile alarm_id_t alarmId{0L};

    static inline AlarmTime alarm1{enabled : false, hour : 0U, minute : 0U};
    static inline AlarmTime alarm2{enabled : false, hour : 0U, minute : 0U};

    static inline volatile State state{State::IDLE};

    void transmit();

    static void cancelTimer();
    static void save();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

    [[nodiscard]] static time_t getEpoch(time_t now, AlarmTime config);

public:
    explicit AlarmExtension() : ExtensionModule(name) {};

    void configure() override;
    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;

    static void dismiss();
    static void reschedule();
    static void setAlarm1(bool enable);
    static void setAlarm2(bool enable);
    static void setTime1(uint8_t hour, uint8_t minute);
    static void setTime2(uint8_t hour, uint8_t minute);
    static void snooze();

    [[nodiscard]] static bool getAlarm1();
    [[nodiscard]] static bool getAlarm2();
};
