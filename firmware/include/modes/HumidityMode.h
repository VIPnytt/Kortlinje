#pragma once

#include "config/IkeaKortlinje.h"
#include "handlers/TextHandler.h"
#include "modules/ModeModule.h"

#include <bits/unique_ptr.h>

class HumidityMode final : public ModeModule
{
private:
    static inline bool pending{false};

    static inline uint8_t humidity{0U};

    static inline unsigned long lastMillis{0UL};

public:
    static constexpr std::string_view name{"Humidity"};

    explicit HumidityMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    static void set(uint8_t _humidity);
};
