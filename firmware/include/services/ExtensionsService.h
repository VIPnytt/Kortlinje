#pragma once

#include "extensions/BatteryExtension.h"
#include "extensions/ButtonsRearExtension.h"
#include "extensions/ButtonsTopExtension.h"
#include "extensions/EnvironmentExtension.h"
#include "extensions/OtaExtension.h"
#include "extensions/PhotocellExtension.h"
#include "extensions/WebSocketExtension.h"
#include "extensions/WebUiExtension.h"
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
    ButtonsRearExtension extensionButtonsRear{};
    ButtonsTopExtension extensionButtonsTop{};
    EnvironmentExtension extensionEnvironment{};
    OtaExtension extensionOta{};
    PhotocellExtension extensionPhotocell{};
    WebSocketExtension extensionWebSocket{};
    WebUiExtension extensionWebUi{};

    const std::array<ExtensionModule *, 8U> modules{
        &extensionBattery,
        &extensionButtonsRear,
        &extensionButtonsTop,
        &extensionEnvironment,
        &extensionOta,
        &extensionPhotocell,
        &extensionWebSocket,
        &extensionWebUi,
    };

public:
    void begin();
    void handle();

    EnvironmentExtension &Environment();

    WebSocketExtension &WebSocket();

    [[nodiscard]] std::span<ExtensionModule *const> getAll();

    static ExtensionsService &getInstance();
};

extern ExtensionsService &Extensions;
