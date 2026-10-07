#include "extensions/OtaExtension.h"

void OtaExtension::begin()
{
    ArduinoOTA.setHostname("kortlinje");
    ArduinoOTA.onProgress(&onProgress);
    ArduinoOTA.onError(&onError);
    ArduinoOTA.begin();
}

void OtaExtension::handle() { ArduinoOTA.handle(); }

void OtaExtension::onError(ota_error_t error)
{
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == ota_error_t::OTA_AUTH_ERROR)
    {
        Serial.println("Auth error");
    }
    else if (error == ota_error_t::OTA_BEGIN_ERROR)
    {
        Serial.println("Begin error");
    }
    else if (error == ota_error_t::OTA_CONNECT_ERROR)
    {
        Serial.println("Connect error");
    }
    else if (error == ota_error_t::OTA_RECEIVE_ERROR)
    {
        Serial.println("Receive error");
    }
    else if (error == ota_error_t::OTA_END_ERROR)
    {
        Serial.println("End error");
    }
}

void OtaExtension::onProgress(unsigned int progress, unsigned int total)
{
    Serial.printf("[OTA] Progress: %u%%\n", (progress / (total / 100U)));
}
