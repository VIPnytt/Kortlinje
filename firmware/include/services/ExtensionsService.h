#pragma once

#include "extensions/BatteryExtension.h"
#include "extensions/ButtonExtension.h"
#include "extensions/ClimateExtension.h"
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
    ButtonExtension extensionButton{};
    ClimateExtension extensionClimate{};
    OtaExtension extensionOta{};
    PhotocellExtension extensionPhotocell{};
    WebSocketExtension extensionWebSocket{};
    WebUiExtension extensionWebUi{};

    const std::array<ExtensionModule *, 7U> modules{
        &extensionBattery,
        &extensionButton,
        &extensionClimate,
        &extensionOta,
        &extensionPhotocell,
        &extensionWebSocket,
        &extensionWebUi,
    };

public:
    void begin();
    void handle();

    ButtonExtension &button();

    WebSocketExtension &WebSocket();

    [[nodiscard]] std::span<ExtensionModule *const> getAll();

    static ExtensionsService &getInstance();
};

extern ExtensionsService &Extensions;
