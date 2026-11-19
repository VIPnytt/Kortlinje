#pragma once

#include "modules/ExtensionModule.h"

class OtaExtension final : public ExtensionModule
{
private:
    void transmit();

    static void onStart();
    static void onProgress(unsigned int progress, unsigned int total);

public:
    static constexpr std::string_view name{"OTA"};

    explicit OtaExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
