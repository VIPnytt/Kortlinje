#pragma once

#include "modules/ExtensionModule.h"
#include "services/WebServerService.h"

#include <ESPAsyncWebServer.h>
#include <pico/time.h>
#include <string_view>

class WebUiExtension final : public ExtensionModule
{
private:
    volatile bool pending{true};

    static inline uint64_t interval{0b1ULL << 16U};

    static void onGetCss(AsyncWebServerRequest *request);
    static void onGetJavascript(AsyncWebServerRequest *request);
    static void onGetSvg(AsyncWebServerRequest *request);
    static void onHtml(AsyncWebServerRequest *request);

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Web UI"};

    explicit WebUiExtension() : ExtensionModule(name) {};

    static inline bool render{false};

    void begin() override;
    void handle() override;
};
