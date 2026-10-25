#pragma once

#include "modules/ExtensionModule.h"
#include "services/WebServerService.h"

#include <ESPAsyncWebServer.h>
#include <array>
#include <pico/time.h>
#include <string_view>

class WebUiExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Web UI"};

    static constexpr auto css{WebServerService::embed({
#embed "../../../webui/dist/index.css.gz"
    })};
    static constexpr auto html{WebServerService::embed({
#embed "../../../webui/dist/index.html.gz"
    })};
    static constexpr auto javascript{WebServerService::embed({
#embed "../../../webui/dist/index.js.gz"
    })};
    static constexpr auto svg{WebServerService::embed({
#embed "../../../webui/dist/favicon.svg.gz"
    })};

    static constexpr std::array<char, 11U> cssEtag{WebServerService::etag(css)};
    static constexpr std::array<char, 11U> htmlEtag{WebServerService::etag(html)};
    static constexpr std::array<char, 11U> javascriptEtag{WebServerService::etag(javascript)};
    static constexpr std::array<char, 11U> svgEtag{WebServerService::etag(svg)};

    static inline volatile bool pending{true};

    static inline uint8_t interval{0U};

    static inline alarm_id_t alarm{0L};

    static void onGetCss(AsyncWebServerRequest *request);
    static void onGetHtml(AsyncWebServerRequest *request);
    static void onGetJavascript(AsyncWebServerRequest *request);
    static void onGetSvg(AsyncWebServerRequest *request);
    static void onHeadRoot(AsyncWebServerRequest *request);

    static int64_t onAlarm(alarm_id_t alarmIdentifier, void *userData);

public:
    explicit WebUiExtension() : ExtensionModule(name) {};

    static inline bool render{false};

    void begin() override;
    void handle() override;
};
