#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <cstdint>
#include <pico/time.h>
#include <string_view>

class FireworkMode final : public ModeModule
{
private:
    volatile bool tick{false};

    uint8_t maxRadius{1U};
    uint8_t radius{0U};
    uint8_t rocketX{0U};
    uint8_t rocketY{Display::grid::rows};
    uint8_t stage{0U};

    alarm_id_t alarmId{0L};

    void pad();
    void launching();
    void exploding();

    void startAlarm(uint64_t delayUs);
    void stopAlarm();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Firework"};

    explicit FireworkMode() : ModeModule(name) {};
    ~FireworkMode() override;

    void begin() override;
    void handle() override;
};
