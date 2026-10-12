#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ButtonExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Button"};

    enum ButtonLevel : uint8_t
    {
        LEVEL0,
        LEVEL1,
        LEVEL2,
        LEVEL3,
        IDLE,
        INVALID,
    };

    bool pressRear{false};
    bool pressTop{false};
    bool toggle{false};

    uint8_t margin{0U};

    uint16_t high{0U};
    uint16_t low{0U};
    uint16_t medium{0U};
    uint16_t idle{0U};

    unsigned long lastMillis{0UL};

    ButtonLevel stateRear{ButtonLevel::INVALID};
    ButtonLevel stateTop{ButtonLevel::INVALID};

    void readRear();
    void rearAlarm();
    void rearDown();
    void rearSet();
    void rearUp();
    void readTop();
    void topAlarm();
    void topBrightness();
    void topCycle();
    void topSnooze();

    [[nodiscard]] ButtonLevel parse(uint16_t raw);

public:
    explicit ButtonExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
