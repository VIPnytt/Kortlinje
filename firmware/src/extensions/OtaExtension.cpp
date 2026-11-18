#include "extensions/OtaExtension.h"

#include "constants/Kortlinje.h"
#include "extensions/MqttExtension.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

#include <hardware/watchdog.h>

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
    DisplayService::fillFrame(false);
    const KortlinjeFont font{};
    TextHandler(std::string("OTA"), font).draw();
    DisplayService::setPower(true);
    MqttExtension::disconnect();
}
