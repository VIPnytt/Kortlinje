#pragma once

#include "modules/ExtensionModule.h"

#include <ArduinoOTA.h>

class OtaExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"OTA"};

    static void onStart();

public:
    explicit OtaExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
