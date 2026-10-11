#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>

class EnvironmentExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Environment"};

    static constexpr uint16_t MAX_EDGES{200U};

    static constexpr uint32_t SYNC_MIN_US{3'200U};
    static constexpr uint32_t SYNC_MAX_US{4'500U};
    static constexpr uint32_t BIT_THRESHOLD_US{1'450U}; // < 1450 us = '0', > 1450 us = '1'

    struct Transition
    {
        uint32_t delta;
        PinStatus level;
    };

    uint8_t humidity{0U};

    float temperature{.0F};

    unsigned long lastMillis{0UL};

    static inline volatile bool capturing{false};

    static inline volatile uint16_t edges{0U};

    static inline volatile uint32_t lastEdge{0U};

    static inline Transition edgeBuffer[MAX_EDGES];

    void read();

    bool parse(uint16_t idxSync, uint8_t payload[5U]);

    static void onChange();

public:
    explicit EnvironmentExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
};
