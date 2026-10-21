#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>
#include <cstdint>

class BatteryExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Battery"};

#ifndef RP2350_ERRATA_E9
    static inline volatile bool pending{false};
#endif // RP2350_ERRATA_E9

#ifdef RP2350_ERRATA_E9
    int8_t debounce{0};
#endif // RP2350_ERRATA_E9

#ifdef RP2350_ERRATA_E9
    unsigned long lastMillis{0UL};
#endif // RP2350_ERRATA_E9

    void sleep();

#ifndef RP2350_ERRATA_E9
    static void onFalling();
#endif // RP2350_ERRATA_E9

public:
    explicit BatteryExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
