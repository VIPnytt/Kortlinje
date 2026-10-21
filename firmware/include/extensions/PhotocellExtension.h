#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>
#include <string_view>

class PhotocellExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Photocell"};

    bool active{true};
    bool detected{false};
    bool pending{true};

    int8_t debounce{0};

    unsigned long lastMillis{0UL};

    void transmit();

public:
    explicit PhotocellExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void setActive(bool _active);

    [[nodiscard]] bool getActive() const;
    [[nodiscard]] bool getDetected() const;
};
