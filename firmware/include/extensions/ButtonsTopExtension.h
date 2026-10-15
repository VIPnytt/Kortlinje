#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ButtonsTopExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Buttons top"};

    enum Button : uint8_t
    {
        NONE,
        BRIGHTNESS,
        CYCLE,
        DISMISS,
        SNOOZE,
        UNKNOWN,
    };

    bool pressed{false};

    uint8_t margin{0U};

    uint16_t high{0U};
    uint16_t low{0U};
    uint16_t medium{0U};
    uint16_t idle{0U};

    unsigned long lastMillis{0UL};

    Button state{Button::UNKNOWN};

    void brightness();
    void cycle();
    void dismiss();
    void snooze();

public:
    explicit ButtonsTopExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
