#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class PhotocellExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Photocell"};

    bool active{true};
    bool detected{true};

    unsigned long lastMillis{0UL};

public:
    explicit PhotocellExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void process();
    void setActive(bool _active);

    [[nodiscard]] bool getActive() const;
};
