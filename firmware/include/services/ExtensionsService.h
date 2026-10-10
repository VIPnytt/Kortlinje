#pragma once

#include "extensions/ButtonExtension.h"
#include "extensions/CdsExtension.h"
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
    OtaExtension extensionOta{};

    const std::array<ExtensionModule *, 3U> modules{
        &extensionButton,
        &extensionCds,
        &extensionOta,
    };

public:
    void begin();
    void handle();

    [[nodiscard]] std::span<ExtensionModule *const> getAll();

    static ExtensionsService &getInstance();
};

extern ExtensionsService &Extensions;
