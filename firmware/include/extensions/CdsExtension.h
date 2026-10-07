#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class CdsExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"CdS"};

    static inline volatile bool changed{false};

    bool state{true};

    int8_t debounce{0};

    static void onChange();

public:
    explicit CdsExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
