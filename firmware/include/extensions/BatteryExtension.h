#pragma once

#include "modules/ExtensionModule.h"

class BatteryExtension final : public ExtensionModule
{
private:
    static inline volatile bool pending{false};

    void sleep();

    static void onInterrupt();
    static void onIrq();

public:
    static constexpr std::string_view name{"Battery"};

    explicit BatteryExtension() : ExtensionModule(name) {};

    void configure() override;
    void begin() override;
    void handle() override;
};
