#include "services/ModesService.h"

#include "constants/HomeAssistant.h"
#include "extensions/MqttExtension.h"
#include "extensions/PlaylistExtension.h"
#include "fonts/SmallFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "modes/AnimationMode.h"
#include "modes/BlindsMode.h"
#include "modes/BlinkMode.h"
#include "modes/BreakoutMode.h"
#include "modes/BrightMode.h"
#include "modes/CircleMode.h"
#include "modes/ClockMode.h"
#include "modes/CountdownMode.h"
#include "modes/DateMode.h"
#include "modes/DrawMode.h"
#include "modes/EpochCounterMode.h"
#include "modes/EqualizerMode.h"
#include "modes/FireworkMode.h"
#include "modes/GameOfLifeMode.h"
#include "modes/GlitterMode.h"
#include "modes/HumidityMode.h"
#include "modes/LeafFallMode.h"
#include "modes/LinesMode.h"
#include "modes/NoiseMode.h"
#include "modes/PingPongMode.h"
#include "modes/RainMode.h"
#include "modes/ScanMode.h"
#include "modes/SnakeMode.h"
#include "modes/StarsMode.h"
#include "modes/TemperatureMode.h"
#include "modes/TickerMode.h"
#include "modes/WaveformMode.h"
#include "modes/WeatherMode.h"
#include "modes/WeekdayMode.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <RP2040Support.h>
#include <array>
#include <memory>
#include <meta>
#include <string>
#include <utility>

void ModesService::begin()
{
    std::string _name{};
    if (!std::ranges::contains(DeviceService::resetAbnormalities, rp2040.getResetReason()) &&
        ConfigHandler::get(name, "mode", _name))
    {
        setMode(_name);
    }
    if (mode == nullptr)
    {
        const std::span<const std::string_view> collection{getCollection()};
        setMode(collection[static_cast<size_t>(random(static_cast<long>(collection.size())))]);
    }
}

void ModesService::handle()
{
    if (!active || mode == nullptr)
    {
        return;
    }
    busy = true;
    if (active)
    {
        if (pending)
        {
            pending = false;
            scheduled = true;
            PlaylistExtension::setEnabled(false);
            DisplayService::fillFrame(false);
            const SmallFont font{};
            TextHandler(std::string(mode->name), font).draw();
            cancelAlarm();
            alarmId = add_alarm_in_us(0b1ULL << 21U, &onAlarm, nullptr, true);
        }
        else if (scheduled)
        {
            if (tick)
            {
                scheduled = false;
                tick = false;
                mode->begin();
                ConfigHandler::set(name, "mode", mode->name);
                transmit();
            }
        }
        else
        {
            mode->handle();
        }
    }
    busy = false;
}

void ModesService::cancelAlarm()
{
    const alarm_id_t _alarmId{alarmId};
    if (_alarmId > 0L)
    {
        alarmId = 0L;
        cancel_alarm(_alarmId);
    }
    tick = false;
}

bool ModesService::getActive() { return active; }

std::span<const std::string_view> ModesService::getCollection()
{
    static constexpr std::array collection{
        AnimationMode::name,
        BlindsMode::name,
        BlinkMode::name,
        BreakoutMode::name,
        BrightMode::name,
        CircleMode::name,
        ClockMode::name,
        CountdownMode::name,
        DateMode::name,
        DrawMode::name,
        EpochCounterMode::name,
        EqualizerMode::name,
        FireworkMode::name,
        GameOfLifeMode::name,
        GlitterMode::name,
        HumidityMode::name,
        LeafFallMode::name,
        LinesMode::name,
        NoiseMode::name,
        PingPongMode::name,
        RainMode::name,
        ScanMode::name,
        SnakeMode::name,
        StarsMode::name,
        TemperatureMode::name,
        TickerMode::name,
        WaveformMode::name,
        WeatherMode::name,
        WeekdayMode::name,
    };
    return collection;
}

ModeModule *ModesService::getMode() { return mode.get(); }

std::unique_ptr<ModeModule> ModesService::getMode(std::string_view modeName)
{
    if (modeName == AnimationMode::name)
    {
        return std::make_unique<AnimationMode>();
    }
    if (modeName == BlindsMode::name)
    {
        return std::make_unique<BlindsMode>();
    }
    if (modeName == BlinkMode::name)
    {
        return std::make_unique<BlinkMode>();
    }
    if (modeName == BreakoutMode::name)
    {
        return std::make_unique<BreakoutMode>();
    }
    if (modeName == BrightMode::name)
    {
        return std::make_unique<BrightMode>();
    }
    if (modeName == CircleMode::name)
    {
        return std::make_unique<CircleMode>();
    }
    if (modeName == ClockMode::name)
    {
        return std::make_unique<ClockMode>();
    }
    if (modeName == CountdownMode::name)
    {
        return std::make_unique<CountdownMode>();
    }
    if (modeName == DateMode::name)
    {
        return std::make_unique<DateMode>();
    }
    if (modeName == DrawMode::name)
    {
        return std::make_unique<DrawMode>();
    }
    if (modeName == EpochCounterMode::name)
    {
        return std::make_unique<EpochCounterMode>();
    }
    if (modeName == EqualizerMode::name)
    {
        return std::make_unique<EqualizerMode>();
    }
    if (modeName == FireworkMode::name)
    {
        return std::make_unique<FireworkMode>();
    }
    if (modeName == GlitterMode::name)
    {
        return std::make_unique<GlitterMode>();
    }
    if (modeName == GameOfLifeMode::name)
    {
        return std::make_unique<GameOfLifeMode>();
    }
    if (modeName == HumidityMode::name)
    {
        return std::make_unique<HumidityMode>();
    }
    if (modeName == LeafFallMode::name)
    {
        return std::make_unique<LeafFallMode>();
    }
    if (modeName == LinesMode::name)
    {
        return std::make_unique<LinesMode>();
    }
    if (modeName == NoiseMode::name)
    {
        return std::make_unique<NoiseMode>();
    }
    if (modeName == PingPongMode::name)
    {
        return std::make_unique<PingPongMode>();
    }
    if (modeName == RainMode::name)
    {
        return std::make_unique<RainMode>();
    }
    if (modeName == ScanMode::name)
    {
        return std::make_unique<ScanMode>();
    }
    if (modeName == SnakeMode::name)
    {
        return std::make_unique<SnakeMode>();
    }
    if (modeName == StarsMode::name)
    {
        return std::make_unique<StarsMode>();
    }
    if (modeName == TemperatureMode::name)
    {
        return std::make_unique<TemperatureMode>();
    }
    if (modeName == TickerMode::name)
    {
        return std::make_unique<TickerMode>();
    }
    if (modeName == WaveformMode::name)
    {
        return std::make_unique<WaveformMode>();
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

int64_t __time_critical_func(ModesService::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<void>(user_data);
    alarmId = 0L;
    tick = true;
    return 0LL;
}

/**
 * @brief Adds Home Assistant discovery metadata for selecting the device mode.
 *
 * @param discovery JSON document to populate with the select component configuration.
 * @param unique Identifier used to create a unique component ID.
 */
void ModesService::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
{
    constexpr std::string_view stateTopic{std::define_static_string(std::string{MqttExtension::topicPrefix} + name)};
    constexpr std::string_view id{std::define_static_string(std::string{name} + "_mode")};
    JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
    component[HomeAssistant::Abbreviations::command_template].set(R"({"mode":"{{value}}"})");
    component[HomeAssistant::Abbreviations::command_topic].set(
        std::define_static_string(std::string{stateTopic} + "/set"));
    component[HomeAssistant::Abbreviations::icon].set("mdi:format-list-bulleted");
    component[HomeAssistant::Abbreviations::name].set("Mode");
    JsonArray options{component[HomeAssistant::Abbreviations::options].to<JsonArray>()};
    for (const std::string_view option : getCollection())
    {
        options.add(option);
    }
    component[HomeAssistant::Abbreviations::platform].set("select");
    component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
    component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
    component[HomeAssistant::Abbreviations::value_template].set("{{value_json.mode}}");
}

void ModesService::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["mode"].is<std::string_view>())
    {
        setMode(payload["mode"].as<std::string_view>());
    }
}

void ModesService::setActive(bool _active)
{
    active = _active;
    if (!_active)
    {
        while (busy)
        {
            tight_loop_contents();
        }
    }
}

void ModesService::setMode(std::string_view modeName, bool power, bool splash)
{
    if (std::unique_ptr<ModeModule> _mode{getMode(modeName)})
    {
        const bool wasActive{active};
        setActive(false);
        cancelAlarm();
        if (mode)
        {
            mode.reset();
        }
        mode = std::move(_mode);
        pending = splash;
        scheduled = !splash;
        tick = !splash;
        if (power)
        {
            DisplayService::setPower(true);
        }
        setActive(wasActive);
    }
}

void ModesService::setModeNext()
{
    const std::span<const std::string_view> collection{getCollection()};
    if (mode == nullptr)
    {
        setMode(collection.front());
    }
    else if (const auto it{std::ranges::find(collection, mode->name)}; it != collection.end())
    {
        const auto next{std::next(it)};
        setMode(*(next == collection.end() ? collection.begin() : next));
    }
}

void ModesService::setModePrevious()
{
    const std::span<const std::string_view> collection{getCollection()};
    if (mode == nullptr)
    {
        setMode(collection.back());
    }
    else if (const auto it{std::ranges::find(collection, mode->name)}; it != collection.end())
    {
        setMode(*std::prev(it == collection.begin() ? collection.end() : it));
    }
}

void ModesService::transmit()
{
    JsonDocument doc{};
    JsonArray collection{doc["collection"].to<JsonArray>()};
    for (const std::string_view _name : getCollection())
    {
        collection.add(_name);
    }
    if (mode != nullptr)
    {
        doc["mode"].set(mode->name);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
