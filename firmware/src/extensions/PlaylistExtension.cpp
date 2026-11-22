#include "extensions/PlaylistExtension.h"

#include "constants/HomeAssistant.h"
#include "extensions/MqttExtension.h"
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
#include <RP2040Support.h>
#include <algorithm>
#include <meta>
#include <string>
#include <utility>
#include <vector>

void PlaylistExtension::begin()
{
    loadSchedule();
    if (!std::ranges::contains(DeviceService::resetAbnormalities, rp2040.getResetReason()) &&
        ConfigHandler::get(name, "enabled", enabled) && enabled)
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
        armTimer(0b1UL << 22U);
        return;
    }
    if (step >= schedule.size())
    {
        step = 0U;
    }
    const Item &item{schedule[step]};
    currentMode = item.mode;
    ModesService::setMode(currentMode, true, false);
    const uint64_t duration{static_cast<uint64_t>(item.duration) * 1'000'000ULL};
    step = (step + 1U) % schedule.size();
    if (duration != 0ULL)
    {
        armTimer(duration);
    }
    else
    {
        tick = true;
    }
}

void PlaylistExtension::armTimer(uint64_t delayUs)
{
    cancelTimer();
    alarmId = add_alarm_in_us(delayUs, &onAlarm, nullptr, true);
}

void PlaylistExtension::cancelTimer()
{
    const alarm_id_t _alarmId{alarmId};
    if (_alarmId > 0L)
    {
        alarmId = 0L;
        cancel_alarm(_alarmId);
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
        if (error == ArduinoJson::V743PB22::DeserializationError::Ok && doc.is<JsonArrayConst>())
        {
            for (const JsonVariantConst item : doc.as<JsonArrayConst>())
            {
                if (item["mode"].is<std::string_view>() && item["duration"].is<uint16_t>())
                {
                    schedule.push_back({
                        .duration = item["duration"].as<uint16_t>(),
                        .mode{item["mode"].as<std::string_view>()},
                    });
                }
            }
        }
    }
    if (schedule.empty())
    {
        schedule = {
            {.duration = 5U, .mode{ClockMode::name}},
            {.duration = 5U, .mode{DateMode::name}},
            {.duration = 5U, .mode{WeekdayMode::name}},
            {.duration = 5U, .mode{TemperatureMode::name}},
            {.duration = 5U, .mode{HumidityMode::name}},
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

/**
 * @brief Adds Home Assistant switch discovery configuration for the playlist.
 *
 * @param discovery JSON document receiving the discovery configuration.
 * @param unique Unique identifier prefix for the Home Assistant entity.
 */
void PlaylistExtension::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
{
    constexpr std::string_view stateTopic{std::define_static_string(std::string{MqttExtension::topicPrefix} + name)};
    constexpr std::string_view id{std::define_static_string(std::string{name} + "_enabled")};
    JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
    component[HomeAssistant::Abbreviations::command_template].set(R"({"enabled":{{value}}})");
    component[HomeAssistant::Abbreviations::command_topic].set(
        std::define_static_string(std::string{stateTopic} + "/set"));
    component[HomeAssistant::Abbreviations::icon].set("mdi:presentation-play");
    component[HomeAssistant::Abbreviations::json_attributes_template].set(
        R"({{{"schedule":value_json.schedule}|to_json}})");
    component[HomeAssistant::Abbreviations::json_attributes_topic].set(stateTopic);
    component[HomeAssistant::Abbreviations::name].set(name);
    component[HomeAssistant::Abbreviations::payload_off].set("false");
    component[HomeAssistant::Abbreviations::payload_on].set("true");
    component[HomeAssistant::Abbreviations::platform].set("switch");
    component[HomeAssistant::Abbreviations::state_off].set("False");
    component[HomeAssistant::Abbreviations::state_on].set("True");
    component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
    component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
    component[HomeAssistant::Abbreviations::value_template].set("{{value_json.enabled}}");
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
                nextItem.push_back({
                    .duration = item["duration"].as<uint16_t>(),
                    .mode{item["mode"].as<std::string_view>()},
                });
            }
        }
        setSchedule(std::move(nextItem));
    }
    if (payload["enabled"].is<bool>())
    {
        setEnabled(payload["enabled"].as<bool>());
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
        JsonObject _item{_schedule.add<JsonObject>()};
        _item["duration"].set(item.duration);
        _item["mode"].set(item.mode);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
