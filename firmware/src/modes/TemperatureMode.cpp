#include "modes/TemperatureMode.h"

#include "constants/HomeAssistant.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

#include <format>

void TemperatureMode::begin()
{
    ConfigHandler::get(name, "temperature", temperature);
    pending = true;
}

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
        if (!saved)
        {
            saved = true;
            ConfigHandler::set(name, "temperature", temperature);
        }
        transmit();
    }
}

void TemperatureMode::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_temperature")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"temperature":{{value}}})");
        component[HomeAssistant::Abbreviations::command_topic].set(topic + "/set");
        component[HomeAssistant::Abbreviations::device_class].set("temperature");
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("config");
        component[HomeAssistant::Abbreviations::icon].set("mdi:thermometer");
        component[HomeAssistant::Abbreviations::max].set(0b1U << 9U);
        component[HomeAssistant::Abbreviations::min].set(-static_cast<int16_t>(0b1U << 9U));
        component[HomeAssistant::Abbreviations::mode].set("box");
        component[HomeAssistant::Abbreviations::name].set(name);
        component[HomeAssistant::Abbreviations::platform].set("number");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::step].set(.1F);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::unit_of_measurement].set(Kortlinje::device::temperature_unit);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.temperature}}");
    }
}

void TemperatureMode::set(float _temperature)
{
    if (_temperature != temperature)
    {
        temperature = _temperature;
        saved = false;
        pending = true;
    }
}

void TemperatureMode::transmit()
{
    JsonDocument doc{};
    doc["temperature"].set(temperature);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
