#include "services/ModesService.h"

#include "constants/HomeAssistant.h"
#include "extensions/PlaylistExtension.h"
#include "fonts/SmallFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <RP2040Support.h>

void ModesService::begin()
{
    std::string _name{};
    const RP2040::resetReason_t reason{rp2040.getResetReason()};
    if (std::ranges::none_of(DeviceService::resetAbnormalities,
                             [&](RP2040::resetReason_t _reason) { return _reason == reason; }) &&
        ConfigHandler::get(name, "mode", _name))
    {
        setMode(_name);
    }
    if (mode == nullptr)
    {
        setMode(names[random(static_cast<long>(names.size()))]);
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
 * @param topic Base MQTT topic for mode commands and state updates.
 * @param unique Identifier used to create a unique component ID.
 */
void ModesService::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_mode")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"mode":"{{value}}"})");
        component[HomeAssistant::Abbreviations::command_topic].set(topic + "/set");
        component[HomeAssistant::Abbreviations::icon].set("mdi:format-list-bulleted");
        component[HomeAssistant::Abbreviations::name].set("Mode");
        JsonArray options{component[HomeAssistant::Abbreviations::options].to<JsonArray>()};
        for (const std::string_view _name : names)
        {
            options.add(_name);
        }
        component[HomeAssistant::Abbreviations::platform].set("select");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.mode}}");
    }
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
    DisplayService::setPower(true);
    if (mode == nullptr)
    {
        return;
    }
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
    if (mode == nullptr)
    {
        return;
    }
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
    JsonArray list{doc["collection"].to<JsonArray>()};
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
