#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class BatteryExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Battery"};

    uint16_t raw{0U};

    unsigned long lastMillis{0UL};

public:
    explicit BatteryExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
