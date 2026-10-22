#pragma once

#include "config/IkeaKortlinje.h"
#include "hardware/pio.h"
#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ClimateExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Climate"};

    int offset{};

    unsigned long lastMillis{0UL};

    static inline volatile bool pending{false};
    static inline volatile bool processing{false};

    static inline uint8_t humidity{0U};

    static inline int stateMachine{};

    static inline float temperature{.0F};

    static inline PIO pio{pio0};

    void transmit();

    static void onInterrupt();

public:
    explicit ClimateExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void sample();

    [[nodiscard]] uint8_t getHumidity() const;

    [[nodiscard]] float getTemperature() const;
};
