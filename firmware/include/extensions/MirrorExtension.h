#pragma once

#include "modules/ExtensionModule.h"

#include <AsyncUDP.h>
#include <cstdint>
#include <pico/time.h>
#include <string_view>

class MirrorExtension final : public ExtensionModule
{
private:
    static constexpr ip_addr_t addr IPADDR4_INIT_BYTES(255U, 255U, 255U, 255U);

    bool enabled{false};
    volatile bool tick{false};

    static inline uint64_t interval{0b1ULL << 16U};

    alarm_id_t alarmId{0L};

    AsyncUDP udp{};

    void transmit();

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *user_data);

public:
    static constexpr std::string_view name{"Mirror"};

    explicit MirrorExtension() : ExtensionModule(name) {};

    static inline bool render{false};

    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
    void onTransmit(JsonObjectConst payload, std::string_view source) override;
    void setEnabled(bool enable);

    [[nodiscard]] bool getEnabled() const;
};
