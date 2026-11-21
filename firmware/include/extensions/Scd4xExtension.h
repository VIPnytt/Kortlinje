#pragma once

#include "modules/ExtensionModule.h"

#include <Wire.h>
#include <span>
#include <string_view>

class Scd4xExtension final : public ExtensionModule
{
private:
    enum class State : uint8_t
    {
        absent,
        stopping,
        measuring,
    };

    static constexpr uint8_t address{0x62U};

    static inline bool pending{false};

    static inline State state{State::absent};

    static inline uint16_t co2{0U};

    static inline uint32_t deadline{0UL};

    static inline std::string_view variant{"SCD4x"};

    void transmit();

    static bool command(uint16_t code);
    static bool read(uint16_t code, std::span<uint16_t> words);

    [[nodiscard]] static uint8_t getCrc(uint8_t msb, uint8_t lsb);
    [[nodiscard]] static TwoWire &getWire();

public:
    static constexpr std::string_view name{"SCD4x"};

    explicit Scd4xExtension() : ExtensionModule(name) {};

    void begin() override;
    void handle() override;
    void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique) override;

    static void resume();
    static void suspend();

    [[nodiscard]] uint16_t getCo2() const;
};
