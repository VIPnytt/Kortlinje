#include "extensions/OtaExtension.h"

#include "constants/board.h"
#include "extensions/MqttExtension.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

#include <ArduinoOTA.h>
#include <hardware/watchdog.h>
#include <pico/platform.h>
#include <string>

void OtaExtension::begin()
{
    ArduinoOTA.setHostname(Kortlinje::device::hostname.data());
    if constexpr (!Kortlinje::ota::password.empty())
    {
        ArduinoOTA.setPasswordHash(Kortlinje::ota::password.data());
    }
    ArduinoOTA.onProgress(&onProgress);
    ArduinoOTA.onStart(&onStart);
    ArduinoOTA.begin(false);
    transmit();
}

void OtaExtension::handle() { ArduinoOTA.handle(); }

void OtaExtension::onProgress(unsigned int progress, unsigned int total)
{
    static_cast<void>(progress);
    static_cast<void>(total);
    watchdog_update();
}

void OtaExtension::onStart()
{
    ModesService::setActive(false);
    DisplayService::fillFrame(false);
    const KortlinjeFont font{};
    TextHandler(std::string("OTA"), font).draw();
    DisplayService::setPower(true);
    MqttExtension::disconnect();
}

void OtaExtension::transmit()
{
    if constexpr (Kortlinje::board::type == "RP2350")
    {
        JsonDocument doc{};
        doc[Kortlinje::board::type].set(0xA0U | rp2350_rom_version());
        DeviceService::transmit(doc.as<JsonObjectConst>(), name);
    }
}
