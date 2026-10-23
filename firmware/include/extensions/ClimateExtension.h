#pragma once

#include "config/IkeaKortlinje.h"
#include "hardware/pio.h"
#include "modules/ExtensionModule.h"

#include <Arduino.h>

class ClimateExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Climate"};

    static inline volatile bool pending{false};

    static inline volatile uint8_t humidity{0U};

    static inline volatile float temperature{.0F};

    void transmit();

    static void onPacer();
    static void onSampler();

public:
    explicit ClimateExtension();

    void begin() override;
    void handle() override;

    [[nodiscard]] uint8_t getHumidity() const;

    [[nodiscard]] float getTemperature() const;
};
