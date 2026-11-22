#include "modes/TemperatureMode.h"

#include "constants/HomeAssistant.h"
#include "extensions/MqttExtension.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/FontsService.h"

#include <format>
#include <meta>
#include <string>

void TemperatureMode::handle()
{
    if (pending)
    {
        pending = false;
        DisplayService::fillFrame(false);
        const KortlinjeFont font{};
        if constexpr (Kortlinje::device::temperature_unit == "°C")
        {
            TextHandler(std::format("{:.1f}°C", temperature), font).draw();
        }
        else if constexpr (Kortlinje::device::temperature_unit == "°F")
        {
            TextHandler(std::format("{:.0f}°F", temperature), font).draw();
        }
        else
        {
            TextHandler(std::format("{:.0f}K", temperature), font).draw();
        }
        transmit();
    }
}

void TemperatureMode::onHomeAssistant(JsonDocument &discovery, std::string_view unique)
{
    constexpr std::string_view stateTopic{std::define_static_string(std::string{MqttExtension::topicPrefix} + name)};
    constexpr std::string_view id{std::define_static_string(std::string{name} + "_temperature")};
    JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
    component[HomeAssistant::Abbreviations::command_template].set(R"({"temperature":{{value}}})");
    component[HomeAssistant::Abbreviations::command_topic].set(
        std::define_static_string(std::string{stateTopic} + "/set"));
    component[HomeAssistant::Abbreviations::device_class].set("temperature");
    component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
    component[HomeAssistant::Abbreviations::entity_category].set("config");
    component[HomeAssistant::Abbreviations::icon].set("mdi:thermometer");
    if constexpr (Kortlinje::device::temperature_unit == "°C")
    {
        component[HomeAssistant::Abbreviations::max].set(99.9F);
        component[HomeAssistant::Abbreviations::min].set(-9.9F);
    }
    else
    {
        component[HomeAssistant::Abbreviations::max].set(999U);
        component[HomeAssistant::Abbreviations::min].set(-99);
    }
    component[HomeAssistant::Abbreviations::mode].set("box");
    component[HomeAssistant::Abbreviations::name].set(name);
    component[HomeAssistant::Abbreviations::platform].set("number");
    component[HomeAssistant::Abbreviations::state_topic].set(stateTopic);
    if constexpr (Kortlinje::device::temperature_unit == "°C")
    {
        component[HomeAssistant::Abbreviations::step].set(.1F);
    }
    component[HomeAssistant::Abbreviations::unique_id].set(std::string{unique} + id);
    component[HomeAssistant::Abbreviations::unit_of_measurement].set(Kortlinje::device::temperature_unit);
    component[HomeAssistant::Abbreviations::value_template].set("{{value_json.temperature}}");
}

void TemperatureMode::set(float _temperature)
{
    if (_temperature != temperature)
    {
        temperature = _temperature;
        pending = true;
    }
}

void TemperatureMode::transmit()
{
    JsonDocument doc{};
    doc["temperature"].set(temperature);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
