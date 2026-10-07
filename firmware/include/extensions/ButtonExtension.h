#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ButtonExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Button"};

    static constexpr uint16_t level1{2'005U};
    static constexpr uint16_t level2{2'650U};
    static constexpr uint16_t level3{2'972U};
    static constexpr uint16_t level4{3'933U};

    static constexpr uint8_t margin{static_cast<uint8_t>((level3 - level2) / 3U)};

    enum ButtonLevel : uint8_t
    {
        LEVEL0,
        LEVEL1,
        LEVEL2,
        LEVEL3,
        LEVEL4,
        INVALID,
    };

    bool pressBack{false};
    bool pressTop{false};
    bool toggle{false};

    ButtonLevel stateBack{ButtonLevel::INVALID};
    ButtonLevel stateTop{ButtonLevel::INVALID};

    void back();
    void backAlarm();
    void backDown();
    void backSet();
    void backUp();
    void top();
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
