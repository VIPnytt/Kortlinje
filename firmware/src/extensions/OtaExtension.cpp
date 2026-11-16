#include "extensions/OtaExtension.h"

#include "constants/Kortlinje.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

void OtaExtension::begin()
{
    ArduinoOTA.setHostname(Kortlinje::device::hostname.data());
    if constexpr (!Kortlinje::ota::password.empty())
    {
        ArduinoOTA.setPasswordHash(Kortlinje::ota::password.data());
    }
    ArduinoOTA.onStart(&onStart);
    ArduinoOTA.begin(false);
}

void OtaExtension::handle() { ArduinoOTA.handle(); }

void OtaExtension::onStart()
{
    DisplayService::fillFrame(false);
    const KortlinjeFont font{};
    TextHandler(std::string("OTA"), font).draw();
}
