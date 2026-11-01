#include "services/WebServerService.h"

#include "extensions/ProvisioningExtension.h"
#include "services/DeviceService.h"

#include <HTTPClient.h>
#include <WiFi.h>

void WebServerService::begin()
{
    http.onNotFound(&onNotFound);
    http.begin();
}

void WebServerService::restart()
{
    http.end();
    delay(20);
    http.begin();
    Serial.println("[web] server restarted");
}

void WebServerService::onEmpty(AsyncWebServerRequest *request) {}

void WebServerService::onNotFound(AsyncWebServerRequest *request)
{
    if (ProvisioningExtension::handleNotFound(request))
    {
        return;
    }
    request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
}
