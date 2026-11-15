#include "extensions/MessageExtension.h"

#include "extensions/MqttExtension.h" // NOLINT(misc-include-cleaner)
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/FontsService.h"
#include "services/ModesService.h"

#include <algorithm>
#include <ranges>

void MessageExtension::begin()
{
    ConfigHandler::get(name, "font", fontName);
    ConfigHandler::get(name, "repeat", repeat);
    if (std::ranges::find(FontsService::names, fontName) == FontsService::names.end())
    {
        fontName = SmallFont::name;
    }
    font = FontsService::get(fontName);
    pending = true;
}

void MessageExtension::handle()
{
    if (pending)
    {
        pending = false;
        font = FontsService::get(fontName);
        if (active)
        {
            text = std::make_unique<TextHandler>(message, *font);
            offsetY = (GRID_ROWS - text->getHeight()) / 2U;
            width = text->getWidth();
        }
        transmit();
    }

    if (reload)
    {
        reload = false;
        if (!active)
        {
            active = true;
            ModesService::setActive(false);
            std::ranges::copy(DisplayService::getFrame(), frame.begin());
            DisplayService::setPower(true);
            if (!timerRunning)
            {
                timerRunning = add_repeating_timer_ms(-static_cast<int32_t>(0b1U << 7U), &onTimer, this, &timer);
            }
        }
        text = std::make_unique<TextHandler>(message, *font);
        offsetX = GRID_COLUMNS;
        offsetY = (GRID_ROWS - text->getHeight()) / 2U;
        width = text->getWidth();
        count = 0U;
    }

    if (active && tick)
    {
        tick = false;

        if (offsetX + width < 0)
        {
            if (++count < repeat)
            {
                offsetX = GRID_COLUMNS;
            }
            else
            {
                if (timerRunning)
                {
                    cancel_repeating_timer(&timer);
                    timerRunning = false;
                    tick = false;
                }
                active = false;
                text.reset();
                DisplayService::setFrame(frame);
                ModesService::setActive(true);
                return;
            }
        }

        DisplayService::fillFrame(false);
        text->draw(offsetX, static_cast<int8_t>(offsetY));
        --offsetX;
    }
}

void MessageExtension::setMessage(std::string_view _message)
{
    if (!_message.empty())
    {
        message = _message;
        reload = true;
        JsonDocument doc{};
        doc["event"].set(message);
        DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
    }
}

void MessageExtension::setFont(std::string_view _fontName)
{
    if (_fontName != fontName && std::ranges::find(FontsService::names, _fontName) != FontsService::names.end())
    {
        fontName = _fontName;
        ConfigHandler::set(name, "font", fontName);
        pending = true;
    }
}

void MessageExtension::setRepeat(uint8_t _repeat)
{
    if (_repeat != repeat)
    {
        repeat = _repeat;
        ConfigHandler::set(name, "repeat", repeat);
        pending = true;
    }
}

void MessageExtension::transmit()
{
    JsonDocument doc{};
    doc["font"].set(fontName);
    doc["repeat"].set(repeat);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}

void MessageExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    // Font
    if (payload["font"].is<std::string_view>())
    {
        setFont(payload["font"].as<std::string_view>());
    }
    // Message
    if (payload["message"].is<std::string_view>())
    {
        setMessage(payload["message"].as<std::string_view>());
    }
    // Repeat
    if (payload["repeat"].is<uint8_t>())
    {
        setRepeat(payload["repeat"].as<uint8_t>());
    }
}

void MessageExtension::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_font")};
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::command_template].set(R"({"font":"{{value}}"})");
        component[HomeAssistantAbbreviations::command_topic].set(topic + "/set");
        component[HomeAssistantAbbreviations::enabled_by_default].set(false);
        component[HomeAssistantAbbreviations::entity_category].set("config");
        component[HomeAssistantAbbreviations::icon].set("mdi:format-font");
        component[HomeAssistantAbbreviations::name].set(std::string(name).append(" font"));
        JsonArray options{component[HomeAssistantAbbreviations::options].to<JsonArray>()};
        for (const std::string_view _name : FontsService::names)
        {
            options.add(_name);
        }
        component[HomeAssistantAbbreviations::platform].set("select");
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::value_template].set("{{value_json.font}}");
    }
    {
        const std::string id{std::string(name)};
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::command_template].set(R"({"message":"{{value}}"})");
        component[HomeAssistantAbbreviations::command_topic].set(topic + "/set");
        component[HomeAssistantAbbreviations::icon].set("mdi:message");
        component[HomeAssistantAbbreviations::name].set(name);
        component[HomeAssistantAbbreviations::platform].set("text");
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::value_template].set("{{value_json.event}}");
    }
    {
        const std::string id{std::string(name).append("_repeat")};
        JsonObject component{discovery[HomeAssistantAbbreviations::components][id].to<JsonObject>()};
        component[HomeAssistantAbbreviations::command_template].set(R"({"repeat":{{value}}})");
        component[HomeAssistantAbbreviations::command_topic].set(topic + "/set");
        component[HomeAssistantAbbreviations::enabled_by_default].set(false);
        component[HomeAssistantAbbreviations::entity_category].set("config");
        component[HomeAssistantAbbreviations::icon].set("mdi:repeat");
        component[HomeAssistantAbbreviations::max].set(5U);
        component[HomeAssistantAbbreviations::min].set(1U);
        component[HomeAssistantAbbreviations::mode].set("slider");
        component[HomeAssistantAbbreviations::name].set(std::string(name).append(" repeat"));
        component[HomeAssistantAbbreviations::platform].set("number");
        component[HomeAssistantAbbreviations::state_topic].set(topic);
        component[HomeAssistantAbbreviations::unique_id].set(unique + id);
        component[HomeAssistantAbbreviations::value_template].set("{{value_json.repeat}}");
    }
}

bool __time_critical_func(MessageExtension::onTimer)(repeating_timer *rt)
{
    static_cast<MessageExtension *>(rt->user_data)->tick = true;
    return true;
}
