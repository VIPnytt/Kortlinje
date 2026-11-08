#pragma once

#include "modules/ExtensionModule.h"
#include "services/WebServerService.h"

#include <ESPAsyncWebServer.h>
#include <pico/time.h>
#include <string_view>

class WebUiExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Web UI"};

    volatile bool pending{true};

    static inline uint32_t interval{0b1U << 6U};

    alarm_id_t alarm{0L};

    static void onGetCss(AsyncWebServerRequest *request);
    static void onGetJavascript(AsyncWebServerRequest *request);
    static void onGetSvg(AsyncWebServerRequest *request);
    static void onHtml(AsyncWebServerRequest *request);

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    explicit WebUiExtension() : ExtensionModule(name) {};

    static inline bool render{false};

    void begin() override;
    void handle() override;
};
