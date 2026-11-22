#include "modes/TickerMode.h"

#include "constants/HomeAssistant.h"
#include "extensions/MqttExtension.h"
#include "fonts/SmallFont.h"
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"
#include "services/FontsService.h"

#include <memory>
#include <meta>
#include <string>

TickerMode::~TickerMode() { stopTimer(); }

void TickerMode::begin()
{
    ConfigHandler::get(name, "message", message);
    std::string fontName{};
    if (ConfigHandler::get(name, "font", fontName))
    {
        setFont(fontName);
    }
    if (font == nullptr)
    {
        setFont(SmallFont::name);
    }
    pending = true;
}

void TickerMode::handle()
{
    if (pending)
    {
        text = std::make_unique<TextHandler>(message, *font);
        offsetX = Display::grid::columns;
        offsetY = (Display::grid::rows - text->getHeight()) / 2U;
        width = static_cast<int16_t>(text->getWidth());
        transmit();
        pending = false;
        (text && width > 0) ? startTimer() : stopTimer();
    }
    else if (tick && text)
    {
        tick = false;
        if (width + offsetX < 0)
        {
            offsetX = Display::grid::columns;
        }
        DisplayService::fillFrame(false);
        text->draw(offsetX, offsetY);
        --offsetX;
    }
}

void TickerMode::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
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
        constexpr std::string_view id{std::define_static_string(std::string{name} + "_text")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"message":"{{value}}"})");
        component[HomeAssistant::Abbreviations::command_topic].set(commandTopic);
        component[HomeAssistant::Abbreviations::icon].set("mdi:message");
        component[HomeAssistant::Abbreviations::name].set(name);
        component[HomeAssistant::Abbreviations::platform].set("text");
        component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
        component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.message}}");
    }
}

void TickerMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["font"].is<std::string_view>())
    {
        setFont(payload["font"].as<std::string_view>());
    }
    if (payload["message"].is<std::string_view>())
    {
        setMessage(payload["message"].as<std::string_view>());
    }
}

bool __time_critical_func(TickerMode::onTimer)(repeating_timer *rt)
{
    static_cast<TickerMode *>(rt->user_data)->tick = true;
    return true;
}

void TickerMode::setFont(std::string_view fontName)
{
    if ((!font || font->name != fontName) && std::ranges::contains(FontsService::getCollection(), fontName))
    {
        font = FontsService::getFont(fontName);
        ConfigHandler::set(name, "font", fontName);
        pending = true;
    }
}

void TickerMode::setMessage(std::string_view _message)
{
    if (_message.length() && _message != message)
    {
        message = _message;
        ConfigHandler::set(name, "message", message);
        pending = true;
    }
}

void TickerMode::startTimer()
{
    if (!timerRunning)
    {
        timerRunning = add_repeating_timer_us(-static_cast<int64_t>(0b1U << 17U), &onTimer, this, &timer);
    }
}

void TickerMode::stopTimer()
{
    if (timerRunning)
    {
        cancel_repeating_timer(&timer);
        timerRunning = false;
        tick = false;
    }
}

void TickerMode::transmit()
{
    JsonDocument doc{};
    doc["font"].set(font->name);
    doc["message"].set(message);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
