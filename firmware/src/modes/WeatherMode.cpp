#include "modes/WeatherMode.h"

#include "fonts/KortlinjeFont.h"
#include "handlers/BitmapHandler.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <WiFi.h>

void WeatherMode::begin()
{
    if (provider == nullptr)
    {
        setProvider(providerNames[0U]);
    }
    lastMillis = millis() - interval;
}

void WeatherMode::handle()
{
    if (WiFi.isConnected() && millis() - lastMillis > interval)
    {
        lastMillis = millis();
        provider->update(condition, temperature, lastMillis);
        if (condition.has_value() && temperature.has_value())
        {
            std::visit(
                [&](auto sign)
                {
                    const BitmapHandler bitmap(sign);
                    const KortlinjeFont font;
#if TEMPERATURE_KELVIN
                    const TextHandler text(std::to_string(temperature.value()), font);
#else
                    const TextHandler text(std::to_string(temperature.value()) + "°", font);
#endif // TEMPERATURE_KELVIN
                    const uint8_t textWidth{text.getWidth()};
                    const uint8_t marginsX{
                        static_cast<uint8_t>(max(0U, GRID_COLUMNS - bitmap.getWidth() - textWidth) / 3U)};
                    DisplayService::fillFrame(false);
                    bitmap.draw(marginsX, (GRID_ROWS - bitmap.getHeight()) / 2U);
                    text.draw(GRID_COLUMNS - textWidth - marginsX - 1U, (GRID_ROWS - text.getHeight()) / 2U);
                },
                provider->getSign(condition.value()));
            transmit();
        }
    }
}

void WeatherMode::setProvider(std::string_view providerName)
{
    if (providerName == OpenMeteoMiddleware::name)
    {
        provider = std::make_unique<OpenMeteoMiddleware>();
    }
    if (providerName == WttrInMiddleware::name)
    {
        provider = std::make_unique<WttrInMiddleware>();
    }
    if (providerName == YrMiddleware::name)
    {
        provider = std::make_unique<YrMiddleware>();
    }
    if (provider)
    {
        condition.reset();
        temperature.reset();
        lastMillis = millis() - interval;
        transmit();
    }
}

void WeatherMode::transmit()
{
    JsonDocument doc{};
    doc["provider"].set(provider->name);
    JsonArray _providers{doc["providers"].to<JsonArray>()};
    for (const std::string_view _provider : providerNames)
    {
        _providers.add(_provider);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}

void WeatherMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["provider"].is<std::string_view>())
    {
        setProvider(payload["provider"].as<std::string_view>());
    }
}
