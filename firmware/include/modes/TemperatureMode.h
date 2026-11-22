#pragma once

#include "modules/ModeModule.h"

#include <string>

class TemperatureMode final : public ModeModule
{
private:
    static inline bool pending{false};

    static inline float temperature{.0F};

    void transmit();

public:
    static constexpr std::string_view name{"Temperature"};

    explicit TemperatureMode() : ModeModule(name) {};

    void handle() override;
    void onHomeAssistant(JsonDocument &discovery, std::string_view unique) override;

    static void set(float _temperature);
};
