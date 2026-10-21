#pragma once

#include "config/secrets.h"
#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ButtonExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Buttons"};

    enum Button : uint8_t
    {
        NONE,
        ALARM,
        BRIGHTNESS,
        CYCLE,
        DISMISS,
        DOWN,
        SET,
        SNOOZE,
        UP,
        UNKNOWN,
    };

    struct Ladder
    {
        const uint pin;
        const Button _high;
        const Button _low;
        const Button _medium;
        const Button _zero;
        volatile Button state;
        bool pressed;
        volatile bool ready;
        uint8_t margin;
        uint16_t high;
        uint16_t idle;
        uint16_t low;
        uint16_t medium;
    };

    static inline volatile uint16_t raw{0U};

    static inline uint channel{0U};

    static inline Ladder rear{PIN_REAR, Button::DOWN, Button::ALARM, Button::UP, Button::SET};
    static inline Ladder top{PIN_TOP, Button::CYCLE, Button::DISMISS, Button::BRIGHTNESS, Button::SNOOZE};
    static inline Ladder *active{&rear};

    void alarm();
    void brightness();
    void config();
    void cycle();
    void dismiss();
    void minus();
    void plus();
    void process(Button button);
    void snooze();

    static void onInterrupt();

public:
    explicit ButtonExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};