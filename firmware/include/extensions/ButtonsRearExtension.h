#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ButtonsRearExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Buttons rear"};

    enum Button : uint8_t
    {
        NONE,
        ALARM,
        CONFIG,
        MINUS,
        PLUS,
        UNKNOWN,
    };

    bool pressed{false};

    uint8_t margin{0U};

    uint16_t high{0U};
    uint16_t low{0U};
    uint16_t medium{0U};
    uint16_t idle{0U};

    uint channel{0U};

    unsigned long lastMillis{0UL};

    Button state{Button::UNKNOWN};

    volatile uint16_t raw{0U};

    void alarm();
    void minus();
    void plus();
    void config();

public:
    explicit ButtonsRearExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
