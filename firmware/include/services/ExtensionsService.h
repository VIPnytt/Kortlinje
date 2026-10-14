#pragma once

#include "extensions/BatteryExtension.h"
#include "extensions/ButtonExtension.h"
#include "extensions/EnvironmentExtension.h"
#include "extensions/OtaExtension.h"
#include "extensions/PhotocellExtension.h"
#include "modules/ExtensionModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <span>

class ExtensionsService final : public ServiceModule
{
private:
    explicit ExtensionsService() : ServiceModule("Extensions") {};

    unsigned long lastMillis{0UL};

    BatteryExtension extensionBattery{};
    ButtonExtension extensionButton{};
    EnvironmentExtension extensionEnvironment{};
    OtaExtension extensionOta{};
    PhotocellExtension extensionPhotocell{};

    const std::array<ExtensionModule *, 5U> modules{
        &extensionBattery,
        &extensionButton,
        &extensionEnvironment,
        &extensionOta,
        &extensionPhotocell,
    };

public:
    void begin();
    void handle();

    EnvironmentExtension &Environment();

    [[nodiscard]] std::span<ExtensionModule *const> getAll();

    static ExtensionsService &getInstance();
};

extern ExtensionsService &Extensions;
