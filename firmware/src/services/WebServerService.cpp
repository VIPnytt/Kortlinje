#include "services/WebServerService.h"

#include "services/DeviceService.h"

#include <HTTPClient.h>
#include <WiFi.h>

void WebServerService::begin()
{
    http.onNotFound(&onNotFound);
    http.begin();
}

void WebServerService::onEmpty(AsyncWebServerRequest *request) {}

void WebServerService::onNotFound(AsyncWebServerRequest *request) { request->send(t_http_codes::HTTP_CODE_NOT_FOUND); }

WebServerService &WebServerService::getInstance()
{
    static WebServerService instance;
    return instance;
}

WebServerService &WebServer{WebServerService::getInstance()};
