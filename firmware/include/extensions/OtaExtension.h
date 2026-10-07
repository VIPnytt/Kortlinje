#pragma once

#include "modules/ExtensionModule.h"

#include <ArduinoOTA.h>

class OtaExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"OTA"};

    static void onError(ota_error_t error);
    static void onProgress(unsigned int progress, unsigned int total);

public:
    explicit OtaExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
