#include "extensions/MqttExtension.h"

#include "constants/HomeAssistant.h"
#include "constants/Kortlinje.h"
#include "services/ConnectivityService.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/ModesService.h"

#include <WiFi.h>
#include <array>
#include <meta>
#include <vector>

/**
 * @brief Publishes the device's Home Assistant MQTT discovery configuration.
 */
void MqttExtension::begin()
{
    client.onMessageAdvanced(&onMessage);
    client.setCleanSession(false);
    client.setWill(std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/availability"),
                   "",
                   true,
                   1);
    client.begin(net);
    client.setHost(Kortlinje::mqtt::host.data(), 1'883);
    if (client.connect(Kortlinje::device::hostname.data(), Kortlinje::mqtt::user.data(), Kortlinje::mqtt::pass.data()))
    {
        onConnect();
    }
    const std::string topic{std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/")};
    const std::string unique{std::format("0x{}_", rp2040.getChipID())};
    JsonDocument discovery{};
    ConnectivityService::onHomeAssistant(discovery, topic, unique);
    DeviceService::onHomeAssistant(discovery, topic, unique);
    DisplayService::onHomeAssistant(discovery, topic, unique);
    ModesService::onHomeAssistant(discovery, topic, unique);
    for (ExtensionModule *extension : ExtensionsService::getAll())
    {
        extension->onHomeAssistant(discovery, topic, unique);
    }
    for (const std::string_view _mode : ModesService::names)
    {
        ModesService::getMode(_mode)->onHomeAssistant(discovery, topic, unique);
    }
    {
        JsonObject availability{discovery[HomeAssistant::Abbreviations::availability].to<JsonObject>()};
        availability[HomeAssistant::Abbreviations::payload_not_available].set("");
        availability[HomeAssistant::Abbreviations::topic].set(
            std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/availability"));
    }
    {
        JsonObject device{discovery[HomeAssistant::Abbreviations::device].to<JsonObject>()};
        device[HomeAssistant::Abbreviations::Device::configuration_url].set(
            std::define_static_string(std::string{"http://"} + Kortlinje::device::hostname + ".local"));
        device[HomeAssistant::Abbreviations::Device::connections][0U][0U].set("mac");
        device[HomeAssistant::Abbreviations::Device::connections][0U][1U].set(WiFi.macAddress());
        device[HomeAssistant::Abbreviations::Device::hw_version].set(ARDUINO_VARIANT);
        device[HomeAssistant::Abbreviations::Device::identifiers][0U].set(std::format("0x{}", rp2040.getChipID()));
        device[HomeAssistant::Abbreviations::Device::manufacturer].set("IKEA");
        device[HomeAssistant::Abbreviations::Device::model].set("Kortlinje");
        device[HomeAssistant::Abbreviations::Device::name].set(Kortlinje::device::name);
        device[HomeAssistant::Abbreviations::Device::sw_version].set(Kortlinje::version);
    }
    {
        JsonObject origin{discovery[HomeAssistant::Abbreviations::origin].to<JsonObject>()};
        origin[HomeAssistant::Abbreviations::Origin::name].set("Kortlinje");
        origin[HomeAssistant::Abbreviations::Origin::support_url].set(
            "https://github.com/VIPnytt/Kortlinje/blob/main/docs/SUPPORT.md");
        origin[HomeAssistant::Abbreviations::Origin::sw_version].set(Kortlinje::version);
    }
    const size_t length{measureJson(discovery)};
    std::vector<char> payload(length + 1U);
    serializeJson(discovery, payload.data(), length + 1U);
    client.publish(discoveryTopic.c_str(), payload.data(), static_cast<int>(length), true, 0);
}

void MqttExtension::handle()
{
    if (client.connected())
    {
        client.loop();
    }
    else if (WiFi.isConnected() && millis() - lastMillis > 0b1UL << 16U)
    {
        lastMillis = millis();
        if (client.connect(
                Kortlinje::device::hostname.data(), Kortlinje::mqtt::user.data(), Kortlinje::mqtt::pass.data()))
        {
            onConnect();
        }
    }
}

/**
 * @brief Disconnects from the MQTT broker after publishing a retained unavailable status.
 */
void MqttExtension::disconnect()
{
    lastMillis = millis();
    if (client.connected())
    {
        client.publish(
            std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/availability"),
            "",
            0,
            true,
            1);
        client.loop();
        client.disconnect();
    }
}

/**
 * @brief Handles a successful MQTT connection.
 *
 * Subscribes to device set commands and publishes the retained online availability status.
 */
void MqttExtension::onConnect()
{
    client.subscribe(std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/+/set"), 2);
    client.publish(std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/availability"),
                   "online",
                   true,
                   1);
}

/**
 * @brief Processes an MQTT message and forwards valid JSON payloads to the device.
 *
 * @param _client MQTT client instance.
 * @param topic Full MQTT topic associated with the message.
 * @param bytes Message payload buffer.
 * @param length Length of the payload buffer in bytes.
 */
void MqttExtension::onMessage(MQTTClient *_client, char *topic, char *bytes, int length)
{
    static_cast<void>(_client);
    JsonDocument doc{};
    if (deserializeJson(doc, bytes, static_cast<size_t>(length)) == DeserializationError::Code::Ok)
    {
        const std::string_view _topic{topic};
        DeviceService::receive(
            doc.as<JsonObjectConst>(), name, _topic.substr(prefixLength, _topic.size() - prefixLength - suffixLength));
    }
}

/**
 * @brief Publishes a device event payload to its MQTT topic.
 *
 * @param payload JSON payload to publish.
 * @param source Device source used to construct the MQTT topic.
 */
void MqttExtension::onTransmit(JsonObjectConst payload, std::string_view source)
{
    const size_t length{measureJson(payload)};
    std::vector<char> message(length + 1U);
    serializeJson(payload, message.data(), length + 1U);
    client.publish(std::string(std::define_static_string(std::string{"kortlinje/"} + Kortlinje::device::hostname + "/"))
                       .append(source)
                       .c_str(),
                   message.data(),
                   static_cast<int>(length),
                   false,
                   payload["event"].isUnbound() ? 0 : 2);
}
