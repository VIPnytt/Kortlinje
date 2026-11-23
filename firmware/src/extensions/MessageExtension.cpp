#include "extensions/MessageExtension.h"

#include "constants/HomeAssistant.h"
#include "extensions/MqttExtension.h"
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"
#include "services/FontsService.h"
#include "services/ModesService.h"

#include <algorithm>
#include <meta>
#include <string>

void MessageExtension::begin()
{
    ConfigHandler::get(name, "font", fontName);
    ConfigHandler::get(name, "repeat", repeat);
    if (!std::ranges::contains(FontsService::getCollection(), fontName))
    {
        fontName = SmallFont::name;
    }
    pending = true;
}

void MessageExtension::handle()
{
    if (pending)
    {
        pending = false;
        if (active && !reload)
        {
            text.emplace(message, *FontsService::getFont(fontName));
            if (text->empty())
            {
                stop();
            }
        }
        transmit();
    }
    if (reload)
    {
        reload = false;
        text.emplace(message, *FontsService::getFont(fontName));
        if (text->empty())
        {
            stop();
            return;
        }
        if (!active)
        {
            active = true;
            ModesService::setActive(false);
            std::ranges::copy(DisplayService::getFrame(), frame.begin());
            DisplayService::setPower(true);
            add_repeating_timer_us(-static_cast<int64_t>(0b1U << 17U), &onTimer, this, &timer);
        }
        step = 1U;
        count = 0U;
        tick = true;
    }
    if (active && tick)
    {
        tick = false;
        if (step >= Display::grid::columns + text->getWidth())
        {
            if (count < repeat)
            {
                ++count;
                step = 0U;
            }
            else
            {
                stop();
                return;
            }
        }
        DisplayService::fillFrame(false);
        text->drawMarquee(step);
        ++step;
    }
}

void MessageExtension::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
{
    constexpr std::string_view stateTopic{std::define_static_string(std::string{MqttExtension::topicPrefix} + name)};
    constexpr std::string_view commandTopic{std::define_static_string(std::string{stateTopic} + "/set")};
    {
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_font")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"font":"{{value}}"})");
        component[HomeAssistant::Abbreviations::command_topic].set(commandTopic);
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("config");
        component[HomeAssistant::Abbreviations::icon].set("mdi:format-font");
        component[HomeAssistant::Abbreviations::name].set(std::define_static_string(std::string{name} + " font"));
        JsonArray options{component[HomeAssistant::Abbreviations::options].to<JsonArray>()};
        for (const std::string_view option : FontsService::getCollection())
        {
            options.add(option);
        }
        component[HomeAssistant::Abbreviations::platform].set("select");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.font}}");
    }
    {
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_repeat")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"repeat":{{value}}})");
        component[HomeAssistant::Abbreviations::command_topic].set(commandTopic);
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("config");
        component[HomeAssistant::Abbreviations::icon].set("mdi:repeat");
        component[HomeAssistant::Abbreviations::min].set(0U);
        component[HomeAssistant::Abbreviations::mode].set("box");
        component[HomeAssistant::Abbreviations::name].set(std::define_static_string(std::string{name} + " repeat"));
        component[HomeAssistant::Abbreviations::platform].set("number");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.repeat}}");
    }
    {
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_text")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"message":"{{value}}"})");
        component[HomeAssistant::Abbreviations::command_topic].set(commandTopic);
        component[HomeAssistant::Abbreviations::icon].set("mdi:message");
        component[HomeAssistant::Abbreviations::name].set(name);
        component[HomeAssistant::Abbreviations::platform].set("text");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.event}}");
    }
}

void MessageExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["font"].is<std::string_view>())
    {
        setFont(payload["font"].as<std::string_view>());
    }
    if (payload["message"].is<std::string_view>())
    {
        setMessage(payload["message"].as<std::string_view>());
    }
    if (payload["repeat"].is<uint8_t>())
    {
        setRepeat(payload["repeat"].as<uint8_t>());
    }
}

bool __time_critical_func(MessageExtension::onTimer)(repeating_timer *rt)
{
    static_cast<MessageExtension *>(rt->user_data)->tick = true;
    return true;
}

void MessageExtension::setFont(std::string_view _fontName)
{
    if (_fontName != fontName && std::ranges::contains(FontsService::getCollection(), _fontName))
    {
        fontName = _fontName;
        ConfigHandler::set(name, "font", fontName);
        pending = true;
    }
}

void MessageExtension::setMessage(std::string_view _message)
{
    if (!_message.empty())
    {
        message = _message;
        reload = true;
        JsonDocument doc{};
        doc["event"].set(_message);
        DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
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

void MessageExtension::stop()
{
    text.reset();
    std::string{}.swap(message);
    if (active)
    {
        cancel_repeating_timer(&timer);
        tick = false;
        active = false;
        DisplayService::setFrame(frame);
        ModesService::setActive(true);
    }
}

void MessageExtension::transmit()
{
    JsonDocument doc{};
    doc["font"].set(fontName);
    doc["repeat"].set(repeat);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}