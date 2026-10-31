#include "extensions/PlaylistExtension.h"

#include "handlers/ConfigHandler.h"
#include "modes/ClockMode.h"
#include "modes/DateMode.h"
#include "modes/HumidityMode.h"
#include "modes/TemperatureMode.h"
#include "modes/WeekdayMode.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <algorithm>

PlaylistExtension::~PlaylistExtension() { cancelTimer(); }

void PlaylistExtension::begin()
{
    ConfigHandler::get(name, "enabled", enabled);
    loadSchedule();
    if (enabled)
    {
        step = 0U;
        advance();
    }
    transmit();
}

void PlaylistExtension::handle()
{
    if (tick)
    {
        tick = false;
        if (enabled)
        {
            advance();
        }
    }
}

void PlaylistExtension::advance()
{
    cancelTimer();
    if (!enabled || schedule.empty())
    {
        setEnabled(false);
        return;
    }
    if (!DisplayService::getPower())
    {
        armTimer(0b1UL << 12U);
        return;
    }
    if (step >= schedule.size())
    {
        step = 0U;
    }
    const Item &item{schedule[step]};
    if (std::ranges::find(ModesService::names, item.mode) != ModesService::names.end())
    {
        currentMode = item.mode;
        ModesService::setMode(currentMode);
    }
    const uint32_t durationMs{static_cast<uint32_t>(item.duration) * 1'000UL};
    step = (step + 1U) % schedule.size();
    if (durationMs > 0U)
    {
        armTimer(durationMs);
    }
    else
    {
        tick = true;
    }
}

void PlaylistExtension::armTimer(uint32_t milliseconds)
{
    cancelTimer();
    alarmId = add_alarm_in_ms(milliseconds, &onAlarm, nullptr, true);
}

void PlaylistExtension::cancelTimer()
{
    if (alarmId > 0L)
    {
        cancel_alarm(alarmId);
        alarmId = 0L;
    }
}

bool PlaylistExtension::getEnabled() { return enabled; }

void PlaylistExtension::loadSchedule()
{
    schedule.clear();
    File file{LittleFS.open(schedulePath, "r")};
    if (file)
    {
        JsonDocument doc{};
        const DeserializationError error{deserializeJson(doc, file)};
        file.close();
        if (ArduinoJson::V743PB22::DeserializationError::Ok && doc.is<JsonArrayConst>())
        {
            for (const JsonVariantConst item : doc.as<JsonArrayConst>())
            {
                if (item["mode"].is<std::string_view>() && item["duration"].is<uint16_t>())
                {
                    const auto _mode{std::ranges::find(ModesService::names, item["mode"].as<std::string_view>())};
                    if (_mode != ModesService::names.end())
                    {
                        schedule.push_back({
                            .mode = *_mode,
                            .duration = item["duration"].as<uint16_t>(),
                        });
                    }
                }
            }
        }
    }
    if (schedule.empty())
    {
        schedule = {
            {.mode = ClockMode::name, .duration = 5U},
            {.mode = DateMode::name, .duration = 5U},
            {.mode = WeekdayMode::name, .duration = 5U},
            {.mode = TemperatureMode::name, .duration = 5U},
            {.mode = HumidityMode::name, .duration = 5U},
        };
        saveSchedule();
    }
}

int64_t __time_critical_func(PlaylistExtension::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<void>(user_data);
    alarmId = 0L;
    tick = true;
    return 0LL;
}

void PlaylistExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["schedule"].is<JsonArrayConst>())
    {
        std::vector<Item> nextItem{};
        for (const JsonVariantConst item : payload["schedule"].as<JsonArrayConst>())
        {
            if (item["mode"].is<std::string_view>() && item["duration"].is<uint16_t>())
            {
                const auto _mode{std::ranges::find(ModesService::names, item["mode"].as<std::string_view>())};
                if (_mode != ModesService::names.end())
                {
                    nextItem.push_back({
                        .mode = *_mode,
                        .duration = item["duration"].as<uint16_t>(),
                    });
                }
            }
        }
        setSchedule(std::move(nextItem));
    }
    if (payload["enabled"].is<bool>())
    {
        setEnabled(payload["enabled"].as<bool>());
    }
}

void PlaylistExtension::onTransmit(JsonObjectConst payload, std::string_view source)
{
    if (enabled && source == ModesService::name && payload["mode"].is<std::string>() &&
        payload["mode"].as<std::string>() != currentMode)
    {
        setEnabled(false);
    }
}

void PlaylistExtension::saveSchedule()
{
    File file{LittleFS.open(scheduleTempPath, "w")};
    if (!file)
    {
        return;
    }
    JsonDocument doc{};
    JsonArray array{doc.to<JsonArray>()};
    for (const Item &item : schedule)
    {
        JsonObject entry{array.add<JsonObject>()};
        entry["mode"].set(item.mode);
        entry["duration"].set(item.duration);
    }
    serializeJson(doc, file);
    file.close();
    LittleFS.remove(schedulePath);
    LittleFS.rename(scheduleTempPath, schedulePath);
}

void PlaylistExtension::setEnabled(bool enable)
{
    if (enable == enabled)
    {
        return;
    }
    enabled = enable;
    ConfigHandler::set(name, "enabled", enabled);
    cancelTimer();
    tick = false;
    if (enabled && !schedule.empty())
    {
        step = 0U;
        advance();
    }
    transmit();
}

void PlaylistExtension::setSchedule(std::vector<Item> items)
{
    schedule = std::move(items);
    saveSchedule();
    if (enabled)
    {
        step = 0U;
        advance();
    }
    transmit();
}

void PlaylistExtension::transmit()
{
    JsonDocument doc{};
    doc["enabled"].set(enabled);
    JsonArray _schedule{doc["schedule"].to<JsonArray>()};
    for (const Item &item : schedule)
    {
        JsonObject obj{_schedule.add<JsonObject>()};
        obj["mode"].set(item.mode);
        obj["duration"].set(item.duration);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
