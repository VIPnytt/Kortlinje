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
    ArduinoOTA.onError(&onError);
    ArduinoOTA.begin();
}

void OtaExtension::handle() { ArduinoOTA.handle(); }

void OtaExtension::onStart()
{
    updating = true;
    DisplayService::fillFrame(false);
    KortlinjeFont font{};
    TextHandler(std::string("OTA"), font).draw();
}

void OtaExtension::onError(ota_error_t error)
{
    Serial.printf("[ota] error %u — resume modes\n", static_cast<unsigned>(error));
    updating = false;
}
