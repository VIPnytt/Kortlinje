#include "extensions/OtaExtension.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

void OtaExtension::begin()
{
    ArduinoOTA.setHostname("kortlinje");
    ArduinoOTA.onStart(&onStart);
    ArduinoOTA.begin();
}

void OtaExtension::handle() { ArduinoOTA.handle(); }

void OtaExtension::onStart()
{
    Display.fillFrame(false);
    KortlinjeFont font{};
    TextHandler(std::string("OTA"), font).draw();
}
