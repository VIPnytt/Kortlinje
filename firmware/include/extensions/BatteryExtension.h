#pragma once

#include "modules/ExtensionModule.h"

#include <ArduinoOTA.h>

class BatteryExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Battery"};

    static inline bool sleep{false};

    static void onFalling();

public:
    explicit BatteryExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
