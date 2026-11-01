#pragma once

#include "modules/ExtensionModule.h"

#include <hardware/pio.h>
#include <pico/time.h>
#include <string_view>

class PhotocellExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Photocell"};

    static inline volatile bool detected{false};
    static inline volatile bool pending{true};
    static inline volatile bool sampled{false};

    static inline volatile alarm_id_t alarm{0L};

    // Off by default: before the first CDS sample, enabled+pending would blank the panel.
    bool enabled{false};

    void transmit();
    static void cancelAlarm();
    static void onPio();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    explicit PhotocellExtension() : ExtensionModule(name) {};

    void configure() override;
    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
    void setEnabled(bool enable);

    [[nodiscard]] bool getEnabled() const;
    [[nodiscard]] bool getDetected() const;
};
