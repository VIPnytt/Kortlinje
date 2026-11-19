#include "modes/HumidityMode.h"

#include "constants/HomeAssistant.h"
#include "fonts/KortlinjeFont.h"
#include "handlers/TextHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/FontsService.h"

void HumidityMode::handle()
{
    if (pending)
    {
        pending = false;
        DisplayService::fillFrame(false);
        const KortlinjeFont font{};
        TextHandler(std::to_string(humidity), font).draw();
        transmit();
    }
}

void HumidityMode::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_temperature")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"humidity":{{value}}})");
        component[HomeAssistant::Abbreviations::command_topic].set(topic + "/set");
        component[HomeAssistant::Abbreviations::device_class].set("humidity");
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::entity_category].set("config");
        component[HomeAssistant::Abbreviations::icon].set("mdi:water-percent");
        component[HomeAssistant::Abbreviations::mode].set("box");
        component[HomeAssistant::Abbreviations::name].set(name);
        component[HomeAssistant::Abbreviations::platform].set("number");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::unit_of_measurement].set("%");
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.humidity}}");
    }
}

void HumidityMode::set(uint8_t _humidity)
{
    if (_humidity != humidity)
    {
        humidity = _humidity;
        pending = true;
    }
}

void HumidityMode::transmit()
{
    JsonDocument doc{};
    doc["humidity"].set(humidity);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
