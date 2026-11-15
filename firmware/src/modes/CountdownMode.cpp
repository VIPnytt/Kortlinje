#include "modes/CountdownMode.h"

#include "extensions/MqttExtension.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <algorithm>
#include <array>
#include <format>
#include <iomanip>
#include <sstream>
#include <string_view>

/**
 * @brief Resets the countdown display state and blinking.
 */
void CountdownMode::begin()
{
    blink = 0U;
    seconds = 0U;
    minutes = 0U;
    transmit();
}

/**
 * @brief Updates the countdown display and handles completion blinking.
 *
 * Redraws the remaining minutes and seconds when they change. When the
 * countdown reaches zero, begins blinking the display and publishes a
 * completion event.
 */
void CountdownMode::handle()
{
    const std::chrono::nanoseconds _nanoseconds{epoch - std::chrono::system_clock::now()};
    const std::chrono::minutes _minutes{std::chrono::duration_cast<std::chrono::minutes>(_nanoseconds)};
    const int64_t absMinutes{_minutes.count()};
    const int64_t absSeconds{std::chrono::duration_cast<std::chrono::seconds>(_nanoseconds - _minutes).count()};
    const uint8_t relMinutes{static_cast<uint8_t>(std::clamp<int64_t>(absMinutes, 0, 99))};
    const uint8_t relSeconds{static_cast<uint8_t>(std::clamp<int64_t>(absMinutes > 99 ? 59 : absSeconds, 0, 59))};
    if (relSeconds != seconds || relMinutes != minutes)
    {
        minutes = relMinutes;
        seconds = relSeconds;
        if (absSeconds >= 0 && absMinutes >= 0)
        {
            DisplayService::fillFrame(false);
            const KortlinjeFont font{};
            TextHandler(std::format("{:02d}:{:02d}", minutes, seconds), font).draw();
            if (absSeconds == 0 && absMinutes == 0)
            {
                blink = 0xFFU;
                odd = true;
                JsonDocument doc{};
                doc["event"].set("done");
                DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
            }
        }
    }
    else if (blink != 0U && odd == static_cast<bool>(static_cast<uint64_t>(absSeconds) & 0b1U))
    {
        --blink;
        odd = !odd;
        DisplayService::invertFrame();
    }
}

/**
 * @brief Adds Home Assistant discovery components for countdown configuration.
 *
 * @param discovery Home Assistant discovery document to populate.
 * @param topic Base command and state topic for the countdown mode.
 * @param unique Unique identifier prefix for the discovered entities.
 */
void CountdownMode::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_timestamp")};
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::command_template].set(R"({"timestamp":"{{value}}"})");
        component[HomeAssistantAbbreviations::command_topic].set(topic + "/set");
        component[HomeAssistantAbbreviations::entity_category].set("config");
        component[HomeAssistantAbbreviations::icon].set("mdi:timer-sand-full");
        component[HomeAssistantAbbreviations::name].set(name);
        component[HomeAssistantAbbreviations::platform].set("time");
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::value_template].set("{{value_json.timestamp}}");
    }
}

/**
 * @brief Updates the countdown target time from a received payload.
 *
 * @param payload Payload containing a duration in seconds or a local clock timestamp (HH:MM:SS).
 */
void CountdownMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["time"].is<uint32_t>())
    {
        epoch = std::chrono::system_clock::now() + std::chrono::seconds(payload["time"].as<uint32_t>());
        blink = 0U;
        transmit();
    }
    else if (payload["timestamp"].is<const char *>())
    {
        const std::chrono::time_point<std::chrono::system_clock> now{std::chrono::system_clock::now()};
        const time_t timer{std::chrono::system_clock::to_time_t(now)};
        tm local{*std::localtime(&timer)};
        std::istringstream stream{payload["timestamp"].as<const char *>()};
        if (stream >> std::get_time(&local, "%T"))
        {
            local.tm_isdst = -1;
            epoch = std::chrono::system_clock::from_time_t(mktime(&local));
            if (epoch < now)
            {
                epoch += std::chrono::days(1);
            }
            blink = 0U;
            transmit();
        }
    }
}

/**
 * @brief Publishes the current countdown target clock timestamp.
 *
 * The timestamp is formatted as local time (HH:MM:SS).
 */
void CountdownMode::transmit()
{
    std::array<char, 9U> buffer{};
    const time_t timer{std::chrono::system_clock::to_time_t(std::max(epoch, std::chrono::system_clock::now()))};
    const tm local{*std::localtime(&timer)};
    const size_t length{strftime(buffer.data(), buffer.size(), "%T", &local)};
    JsonDocument doc{};
    doc["timestamp"].set(std::string_view(buffer.data(), std::min(buffer.size(), length)));
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
