#pragma once

#include "modules/ModeModule.h"

class Co2Mode final : public ModeModule
{
private:
    static inline bool pending{false};

    static inline uint16_t co2{0U};

    void transmit();

public:
    static constexpr std::string_view name{"CO2"};

    explicit Co2Mode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    static void set(uint16_t _co2);
};
