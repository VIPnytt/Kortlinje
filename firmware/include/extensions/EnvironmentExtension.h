#pragma once

#include "modules/ExtensionModule.h"

#include <Arduino.h>
#include <array>
#include <span>

class EnvironmentExtension final : public ExtensionModule
{
private:
    static constexpr std::string_view name{"Environment"};

    struct Transition
    {
        uint32_t delta;
        PinStatus level;
    };

    bool pending{false};

    uint8_t humidity{0U};

    float temperature{.0F};

    unsigned long lastMillis{0UL};

    static inline volatile bool capturing{false};

    static inline volatile uint16_t edges{0U};

    static inline volatile uint32_t lastEdge{0U};

    static inline std::array<Transition, 200U> edgeBuffer;

    void poll();

    bool parse(uint16_t idxSync, std::span<uint8_t, 5U> payload);

    static void onChange();

public:
    explicit EnvironmentExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void queue();
};
