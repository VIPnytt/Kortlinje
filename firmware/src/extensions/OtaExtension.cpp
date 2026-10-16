#include "extensions/OtaExtension.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DisplayService.h"

void OtaExtension::begin()
{
    ArduinoOTA.setHostname(HOSTNAME);
#ifdef OTA_KEY
    ArduinoOTA.setPasswordHash(OTA_KEY);
#endif // OTA_KEY
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
