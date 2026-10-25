#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>
#include <hardware/pio.h>
#include <pico/time.h>
#include <string_view>

class PhotocellExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Photocell"};

    static inline volatile bool detected{false};
    static inline volatile bool pending{true};

    static inline volatile alarm_id_t alarm{0L};

    bool enabled{true};

    void transmit();
    static void cancelAlarm();
    static void onPio();

    static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    explicit PhotocellExtension();

    void begin() override;
    void handle() override;
    void setEnabled(bool enable);

    [[nodiscard]] bool getEnabled() const;
    [[nodiscard]] bool getDetected() const;
};
