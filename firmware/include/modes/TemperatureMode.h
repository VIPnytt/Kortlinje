#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ModeModule.h"

#include <Arduino.h>

class TemperatureMode final : public ModeModule
{
private:
    static inline bool pending{false};

    static inline float temperature{.0F};

    static inline unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Temperature"};

    explicit TemperatureMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    static void set(float _temperature);
};
