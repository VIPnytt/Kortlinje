#include "services/ModesService.h"

#include "extensions/OtaExtension.h"
#include "extensions/ProvisioningExtension.h"
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <vector>

void ModesService::begin()
{
    std::string _name{};
    if (ConfigHandler::get(name, "mode", _name))
    {
        setMode(_name);
    }
    if (mode == nullptr)
    {
        setMode(names[random(names.size())]);
    }
}

void ModesService::handle()
{
    if (OtaExtension::isUpdating())
    {
        // Keep the OTA frame; DeviceService still flushes.
        return;
    }
    static bool provisioningShown{false};
    if (ProvisioningExtension::isActive())
    {
        provisioningShown = true;
        ProvisioningExtension::draw();
        return;
    }
    if (provisioningShown && mode != nullptr)
    {
        provisioningShown = false;
        mode->begin(); // re-arm pending so modes that only redraw on change repaint
    }
    if (mode != nullptr)
    {
        mode->handle();
    }
}

std::unique_ptr<ModeModule> ModesService::getMode(std::string_view modeName)
{
    if (modeName == ClockMode::name)
    {
        return std::make_unique<ClockMode>();
    }
    if (modeName == DateMode::name)
    {
        return std::make_unique<DateMode>();
    }
    if (modeName == EqualizerMode::name)
    {
        return std::make_unique<EqualizerMode>();
    }
    if (modeName == HumidityMode::name)
    {
        return std::make_unique<HumidityMode>();
    }
    if (modeName == PingPongMode::name)
    {
        return std::make_unique<PingPongMode>();
    }
    if (modeName == SnakeMode::name)
    {
        return std::make_unique<SnakeMode>();
    }
    if (modeName == TemperatureMode::name)
    {
        return std::make_unique<TemperatureMode>();
    }
    if (modeName == TickerMode::name)
    {
        return std::make_unique<TickerMode>();
    }
    if (modeName == WeatherMode::name)
    {
        return std::make_unique<WeatherMode>();
    }
    if (modeName == WeekdayMode::name)
    {
        return std::make_unique<WeekdayMode>();
    }
    return nullptr;
}

void ModesService::setMode(std::string_view modeName, bool power)
{
    if (std::unique_ptr<ModeModule> _mode{getMode(modeName)})
    {
        if (mode)
        {
            mode->end();
            mode.reset();
        }
        mode = std::move(_mode);
        if (power)
        {
            DisplayService::setPower(true);
        }
        mode->begin();
        ConfigHandler::set(name, "mode", modeName);
        transmit();
    }
}

ModeModule *ModesService::getMode() { return mode.get(); }

void ModesService::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["mode"].is<std::string_view>())
    {
        setMode(payload["mode"].as<std::string_view>());
    }
}

void ModesService::setModeNext()
{
    DisplayService::setPower(true);
    for (size_t idx{0U}; idx < names.size(); ++idx)
    {
        if (names[idx] == mode->name)
        {
            const size_t index{(idx + 1U) % names.size()};
            setMode(names[index], index != 0U);
            return;
        }
    }
}

void ModesService::setModePrevious()
{
    DisplayService::setPower(true);
    for (size_t idx{0U}; idx < names.size(); ++idx)
    {
        if (names[idx] == mode->name)
        {
            const size_t index{(idx + names.size() - 1U) % names.size()};
            setMode(names[index], index != 0U);
            return;
        }
    }
}

void ModesService::transmit()
{
    JsonDocument doc{};
    JsonArray list{doc["list"].to<JsonArray>()};
    for (const std::string_view _name : names)
    {
        list.add(_name);
    }
    if (mode != nullptr)
    {
        doc["mode"].set(mode->name);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
