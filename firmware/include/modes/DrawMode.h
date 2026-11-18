#pragma once

#include "modules/ModeModule.h"
#include "services/DisplayService.h"

#include <array>
#include <cstdint>
#include <string_view>

class DrawMode final : public ModeModule
{
private:
    bool pending{false};
    bool render{false};

    std::array<uint8_t, Display::grid::bytes> frame{};

    void load(bool cache = false);
    void save(bool cache = false);
    void transmit();

public:
    static constexpr std::string_view name{"Draw"};

    explicit DrawMode() : ModeModule(name) {};
    ~DrawMode() override;

    void begin() override;
    void handle() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
