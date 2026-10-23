#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>
#include <hardware/pio.h>
#include <string_view>

class PhotocellExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Photocell"};

    bool detected{false};
    bool enabled{true};
    static inline volatile bool pending{true};
    static inline volatile bool state{false};

    static inline volatile unsigned long lastMillis{0UL};

    void transmit();

    static void onInterrupt();

public:
    explicit PhotocellExtension();

    void begin() override;
    void handle() override;
    void setEnabled(bool enable);

    [[nodiscard]] bool getEnabled() const;
    [[nodiscard]] bool getDetected() const;
};
