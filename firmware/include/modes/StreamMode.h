#pragma once

#include "modules/ModeModule.h"

#include <AsyncUDP.h>
#include <string_view>

class StreamMode final : public ModeModule
{
private:
    AsyncUDP udp{};

    static void onDistributedDisplayProtocol(AsyncUDPPacket packet);

public:
    static constexpr std::string_view name{"Stream"};

    explicit StreamMode() : ModeModule(name) {};
    ~StreamMode() override;

    void begin() override;
};
