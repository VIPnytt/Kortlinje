#include "services/ExtensionsService.h"

#include "extensions/AlarmExtension.h"
#include "extensions/AlexaExtension.h"
#include "extensions/BatteryExtension.h"
#include "extensions/ButtonExtension.h"
#include "extensions/ClimateExtension.h"
#include "extensions/MessageExtension.h"
#include "extensions/MirrorExtension.h"
#include "extensions/MqttExtension.h"
#include "extensions/OtaExtension.h"
#include "extensions/PhotocellExtension.h"
#include "extensions/PlaylistExtension.h"
#include "extensions/RestExtension.h"
#include "extensions/SignExtension.h"
#include "extensions/WebSocketExtension.h"
#include "extensions/WebUiExtension.h"

#include <array>

namespace Extensions
{
AlarmExtension alarm{};
AlexaExtension alexa{};
BatteryExtension battery{};
ButtonExtension button{};
ClimateExtension climate{};
MessageExtension message{};
MirrorExtension mirror{};
MqttExtension mqtt{};
OtaExtension ota{};
PhotocellExtension photocell{};
PlaylistExtension playlist{};
RestExtension rest{};
SignExtension sign{};
WebSocketExtension webSocket{};
WebUiExtension webUi{};
} // namespace Extensions

void ExtensionsService::configure()
{
    for (ExtensionModule *extension : getAll())
    {
        extension->configure();
    }
}

void ExtensionsService::begin()
{
    for (ExtensionModule *extension : getAll())
    {
        extension->begin();
    }
}

void ExtensionsService::handle()
{
    for (ExtensionModule *extension : getAll())
    {
        extension->handle();
    }
}

std::span<ExtensionModule *const> ExtensionsService::getAll()
{
    static constexpr std::array<ExtensionModule *, 15U> modules{
        &Extensions::alarm,
        &Extensions::alexa,
        &Extensions::battery,
        &Extensions::button,
        &Extensions::climate,
        &Extensions::message,
        &Extensions::mirror,
        &Extensions::mqtt,
        &Extensions::ota,
        &Extensions::photocell,
        &Extensions::playlist,
        &Extensions::rest,
        &Extensions::sign,
        &Extensions::webSocket,
        &Extensions::webUi,
    };
    return modules;
}

ExtensionModule *const ExtensionsService::getExtension(std::string_view extensionName)
{
    if (extensionName == AlarmExtension::name)
    {
        return &Extensions::alarm;
    }
    if (extensionName == AlexaExtension::name)
    {
        return &Extensions::alexa;
    }
    if (extensionName == BatteryExtension::name)
    {
        return &Extensions::battery;
    }
    if (extensionName == ButtonExtension::name)
    {
        return &Extensions::button;
    }
    if (extensionName == ClimateExtension::name)
    {
        return &Extensions::climate;
    }
    if (extensionName == MessageExtension::name)
    {
        return &Extensions::message;
    }
    if (extensionName == MirrorExtension::name)
    {
        return &Extensions::mirror;
    }
    if (extensionName == MqttExtension::name)
    {
        return &Extensions::mqtt;
    }
    if (extensionName == OtaExtension::name)
    {
        return &Extensions::ota;
    }
    if (extensionName == PhotocellExtension::name)
    {
        return &Extensions::photocell;
    }
    if (extensionName == PlaylistExtension::name)
    {
        return &Extensions::playlist;
    }
    if (extensionName == RestExtension::name)
    {
        return &Extensions::rest;
    }
    if (extensionName == SignExtension::name)
    {
        return &Extensions::sign;
    }
    if (extensionName == WebSocketExtension::name)
    {
        return &Extensions::webSocket;
    }
    if (extensionName == WebUiExtension::name)
    {
        return &Extensions::webUi;
    }
    return nullptr;
}
