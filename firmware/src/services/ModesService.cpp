#include "services/ModesService.h"

#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <vector>

void ModesService::begin()
{
    if (mode == nullptr)
    {
        setMode(names[random(names.size())]);
    }
}

void ModesService::handle()
{
    if (mode != nullptr)
    {
        mode->handle();
    }
}

std::unique_ptr<ModeModule> ModesService::getMode(std::string_view modeName)
{
    if (modeName == EqualizerMode::name)
    {
        return std::make_unique<EqualizerMode>();
    }
    if (modeName == SnakeMode::name)
    {
        return std::make_unique<SnakeMode>();
    }
    if (modeName == TickerMode::name)
    {
        return std::make_unique<TickerMode>();
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
            Display.setPower(true);
        }
        mode->begin();
    }
}

ModeModule *ModesService::getMode() { return mode.get(); }

void ModesService::setModeNext()
{
    Display.setPower(true);
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
    Display.setPower(true);
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

ModesService &ModesService::getInstance()
{
    static ModesService instance;
    return instance;
}

ModesService &Modes{ModesService::getInstance()};
