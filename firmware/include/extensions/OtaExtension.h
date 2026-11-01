#pragma once

#include "modules/ExtensionModule.h"

#include <ArduinoOTA.h>

class OtaExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"OTA"};

    static inline bool updating{false};

    static void onStart();
    static void onError(ota_error_t error);

public:
    explicit OtaExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;

    [[nodiscard]] static bool isUpdating() { return updating; }
};
