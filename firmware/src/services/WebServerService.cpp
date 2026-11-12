#include "services/WebServerService.h"

#include "services/DeviceService.h"

#include <HTTPClient.h>

void WebServerService::begin()
{
    http.onNotFound(&onNotFound);
    http.begin();
}

void WebServerService::onEmpty(AsyncWebServerRequest *request) {}

void WebServerService::onNotFound(AsyncWebServerRequest *request)
{
    if (WiFi.getMode() == WIFI_AP)
    {
        request->redirect("http://" + WiFi.softAPIP().toString() + "/#/connectivity");
    }
    else
    {
        request->send(t_http_codes::HTTP_CODE_NOT_FOUND);
    }
}
