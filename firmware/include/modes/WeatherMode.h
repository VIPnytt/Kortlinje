#pragma once

#include "middlewares/OpenMeteoMiddleware.h"
#include "middlewares/WttrInMiddleware.h"
#include "middlewares/YrMiddleware.h"
#include "modules/ModeModule.h"

#include <array>
#include <memory>
#include <optional>

class WeatherMode final : public ModeModule
{
private:
    static constexpr std::array<std::string_view, 9U> conditionNames{
        "Clear",
        "Cloudy",
        "Partly cloudy",
        "Exception",
        "Fog",
        "Rain",
        "Snow",
        "Thunder",
        "Wind",
    };

    static constexpr std::array<std::string_view, 3U> providerNames{
        OpenMeteoMiddleware::name,
        WttrInMiddleware::name,
        YrMiddleware::name,
    };

    unsigned long lastMillis{0UL};

    std::optional<WeatherHandler::Condition> condition{};

    std::optional<int16_t> temperature{};

    std::unique_ptr<WeatherHandler> provider{};

    void setProvider(std::string_view providerName);

    void transmit();

public:
    static constexpr std::string_view name{"Weather"};

    static constexpr unsigned long interval{0b1UL << 19U};

    explicit WeatherMode() : ModeModule(name) {};

    void begin() override;
    void handle() override;

    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
