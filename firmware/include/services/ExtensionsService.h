#pragma once

#include "extensions/ButtonExtension.h"
#include "extensions/CdsExtension.h"
#include "extensions/EnvironmentExtension.h"
#include "extensions/OtaExtension.h"
#include "modules/ExtensionModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <span>

class ExtensionsService final : public ServiceModule
{
private:
    explicit ExtensionsService() : ServiceModule("Extensions") {};

    unsigned long lastMillis{0UL};

    ButtonExtension extensionButton{};
    CdsExtension extensionCds{};
    EnvironmentExtension extensionEnvironment{};
    OtaExtension extensionOta{};

    const std::array<ExtensionModule *, 4U> modules{
        &extensionButton,
        &extensionCds,
        &extensionEnvironment,
        &extensionOta,
    };

public:
    void begin();
    void handle();

    [[nodiscard]] std::span<ExtensionModule *const> getAll();

    static ExtensionsService &getInstance();
};

extern ExtensionsService &Extensions;
