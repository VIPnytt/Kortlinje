#pragma once

#include "config/IkeaKortlinje.h"
#include "modules/ExtensionModule.h"

#include <AsyncUDP.h>
#include <cstdint>
#include <pico/time.h>
#include <string_view>


class MirrorExtension final : public ExtensionModule
{
private:
    static constexpr size_t bytes{((GRID_COLUMNS * GRID_ROWS) + 7U) >> 3U};

    static constexpr ip_addr_t addr IPADDR4_INIT_BYTES(255, 255, 255, 255);

    bool enabled{false};
    volatile bool pending{false};

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