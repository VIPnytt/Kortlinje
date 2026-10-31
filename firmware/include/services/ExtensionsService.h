#pragma once

#include "extensions/AlarmExtension.h"
#include "extensions/BatteryExtension.h"
#include "extensions/ButtonExtension.h"
#include "extensions/ClimateExtension.h"
#include "extensions/OtaExtension.h"
#include "extensions/PhotocellExtension.h"
#include "extensions/PlaylistExtension.h"
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

    AlarmExtension extensionAlarm{};
    BatteryExtension extensionBattery{};
    ButtonExtension extensionButton{};
    ClimateExtension extensionClimate{};
    OtaExtension extensionOta{};
    PhotocellExtension extensionPhotocell{};
    PlaylistExtension extensionPlaylist{};
    WebSocketExtension extensionWebSocket{};
    WebUiExtension extensionWebUi{};

    const std::array<ExtensionModule *, 9U> modules{
        &extensionAlarm,
        &extensionBattery,
        &extensionButton,
        &extensionClimate,
        &extensionOta,
        &extensionPhotocell,
        &extensionPlaylist,
        &extensionWebSocket,
        &extensionWebUi,
    };

public:
    void begin();
    void configure();
    void handle();

    [[nodiscard]] std::span<ExtensionModule *const> getAll();

    static ExtensionsService &getInstance();
};

extern ExtensionsService &Extensions;
