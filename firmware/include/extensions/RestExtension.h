#pragma once

#include "modules/ExtensionModule.h"

#include <ESPAsyncWebServer.h>
#include <vector>

class RestExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Rest"};

    static constexpr size_t prefixLength{sizeof("/rest/") - 1ULL};

    static inline std::vector<uint8_t> buffer{};

    static void onGet(AsyncWebServerRequest *request);
    static void onPatch(AsyncWebServerRequest *request, const uint8_t *data, size_t len, size_t index, size_t total);

public:
    explicit RestExtension() : ExtensionModule(name) {};

    void begin() override;
};
