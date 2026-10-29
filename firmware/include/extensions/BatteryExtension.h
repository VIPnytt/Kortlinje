#pragma once

#include "modules/ExtensionModule.h"

#include <hardware/pio.h>

class BatteryExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Battery"};

    static inline volatile bool pending{false};

    void sleep();

    static void onInterrupt();

public:
    explicit BatteryExtension() : ExtensionModule(name) {};

    void configure() override;
    void begin() override;
    void handle() override;
};
