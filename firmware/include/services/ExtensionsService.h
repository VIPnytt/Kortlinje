#pragma once

#include "extensions/AlarmExtension.h"
#include "extensions/BatteryExtension.h"
#include "extensions/ButtonExtension.h"
#include "extensions/ClimateExtension.h"
#include "extensions/OtaExtension.h"
#include "extensions/PhotocellExtension.h"
#include "extensions/PlaylistExtension.h"
#include "extensions/ProvisioningExtension.h"
#include "extensions/WebSocketExtension.h"
#include "extensions/WebUiExtension.h"
#include "modules/ExtensionModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <span>

class ExtensionsService final : public ServiceModule
{
private:
    static constexpr std::string_view name{"Extensions"};

    unsigned long lastMillis{0UL};

    static inline AlarmExtension extensionAlarm{};
    static inline BatteryExtension extensionBattery{};
    static inline ButtonExtension extensionButton{};
    static inline ClimateExtension extensionClimate{};
    static inline OtaExtension extensionOta{};
    static inline PhotocellExtension extensionPhotocell{};
    static inline PlaylistExtension extensionPlaylist{};
    static inline ProvisioningExtension extensionProvisioning{};
    static inline WebSocketExtension extensionWebSocket{};
    static inline WebUiExtension extensionWebUi{};

    static inline const std::array<ExtensionModule *, 10U> modules{
        &extensionAlarm,
        &extensionBattery,
        &extensionButton,
        &extensionClimate,
        &extensionOta,
        &extensionPhotocell,
        &extensionPlaylist,
        &extensionProvisioning,
        &extensionWebSocket,
        &extensionWebUi,
    };

public:
    explicit ExtensionsService() : ServiceModule(name) {};

    void begin();
    void configure();
    void handle();

    [[nodiscard]] static std::span<ExtensionModule *const> getAll();
};
