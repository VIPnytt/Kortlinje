#pragma once

#include "config/secrets.h"
#include "modules/ExtensionModule.h"

#include <hardware/dma.h>

class ButtonExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Buttons"};

    static constexpr uint channel{NUM_DMA_CHANNELS - 1U};

    static constexpr uint32_t count{0b1U << 3U};

    static constexpr float divider{static_cast<float>(0xFFFFU)};

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
        bool pressed{false};
        volatile bool ready{false};
        uint8_t margin{0U};
        uint16_t high{0U};
        uint16_t idle{0U};
        uint16_t low{0U};
        uint16_t medium{0U};
        const uint pin{0U};
        const Button _high{Button::UNKNOWN};
        const Button _low{Button::UNKNOWN};
        const Button _medium{Button::UNKNOWN};
        const Button _zero{Button::UNKNOWN};
        volatile Button state{Button::UNKNOWN};
    };

    static inline volatile bool enabled{true};

    static inline uint16_t bursts{0U};
    static inline volatile uint16_t raw{0U};

    static inline Ladder rear{
        pressed : false,
        ready : false,
        margin : 0U,
        high : 0U,
        idle : 0U,
        low : 0U,
        medium : 0U,
        pin : PIN_REAR,
        _high : Button::DOWN,
        _low : Button::ALARM,
        _medium : Button::UP,
        _zero : Button::SET,
        state : Button::NONE,
    };
    static inline Ladder top{
        pressed : false,
        ready : false,
        margin : 0U,
        high : 0U,
        idle : 0U,
        low : 0U,
        medium : 0U,
        pin : PIN_TOP,
        _high : Button::CYCLE,
        _low : Button::DISMISS,
        _medium : Button::BRIGHTNESS,
        _zero : Button::SNOOZE,
        state : Button::NONE,
    };
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

    void configure() override;
    void begin() override;
    void handle() override;

    static void enable();

    [[nodiscard]] static bool isEnabled();
};
