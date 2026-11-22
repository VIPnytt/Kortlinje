#include "extensions/MqttExtension.h"

#include "constants/HomeAssistant.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

#include <WiFi.h>
#include <array>
#include <cstring>
#include <string>
#include <vector>

/**
 * @brief Configures the MQTT client and Last Will and Testament.
 */
void MqttExtension::begin()
{
    if constexpr (!Kortlinje::mqtt::host.empty())
    {
        net.setNoDelay(true);
        client.onMessageAdvanced(&onMessage);
        client.setCleanSession(false);
        client.setWill(availabilityTopic.data(), "", true, 1);
        client.begin(net);
        client.setHost(Kortlinje::mqtt::host.data(), 1'883);
    }
}

void MqttExtension::handle()
{
    if constexpr (!Kortlinje::mqtt::host.empty())
    {
        if (client.connected())
        {
            client.loop();
        }
        else if (tick && WiFi.isConnected())
        {
            tick = false;
            add_alarm_in_us(0b1ULL << 26U, &onAlarm, this, true);
            if (client.connect(Kortlinje::device::hostname.data(),
                               Kortlinje::mqtt::username.data(),
                               Kortlinje::mqtt::password.data()))
            {
                client.subscribe(commandTopic.data(), 2);
                client.publish(availabilityTopic.data(), "online", true, 1);
                if (!discovered)
                {
                    discover();
                }
            }
        }
    }
}

/**
 * @brief Disconnects from the MQTT broker after publishing a retained unavailable status.
 */
void MqttExtension::disconnect()
{
    if constexpr (!Kortlinje::mqtt::host.empty())
    {
        if (client.connected())
        {
            client.publish(availabilityTopic.data(), "", 0, true, 1);
            client.loop();
            client.disconnect();
        }
    }
}

/**
 * @brief Builds and publishes the device's Home Assistant MQTT discovery configuration.
 */
void MqttExtension::discover()
{
    const std::string topic{topicPrefix};
    const std::string unique{std::format("0x{}_", rp2040.getChipID())};
    JsonDocument discovery{};
    ConnectivityService::onHomeAssistant(discovery, unique);
    DeviceService::onHomeAssistant(discovery, unique);
    DisplayService::onHomeAssistant(discovery, unique);
    ModesService::onHomeAssistant(discovery, unique);
    for (ExtensionModule *extension : ExtensionsService::getAll())
    {
        extension->onHomeAssistant(discovery, unique);
    }
    for (const std::string_view _name : ModesService::getCollection())
    {
        ModesService::getMode(_name)->onHomeAssistant(discovery, unique);
    }
    JsonObject availability{discovery[HomeAssistant::Abbreviations::availability].to<JsonObject>()};
    availability[HomeAssistant::Abbreviations::payload_not_available].set("");
    availability[HomeAssistant::Abbreviations::topic].set(availabilityTopic);
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
    JsonObject origin{discovery[HomeAssistant::Abbreviations::origin].to<JsonObject>()};
    origin[HomeAssistant::Abbreviations::Origin::name].set("Kortlinje");
    origin[HomeAssistant::Abbreviations::Origin::support_url].set("https://github.com/VIPnytt/Kortlinje/discussions");
    origin[HomeAssistant::Abbreviations::Origin::sw_version].set(Kortlinje::version);
    const size_t length{measureJson(discovery)};
    std::vector<char> payload(length + 1U);
    serializeJson(discovery, payload.data(), payload.size());
    net.flush();
    discovered = client.publish(std::format("homeassistant/device/0x{}/config", rp2040.getChipID()).c_str(),
                                payload.data(),
                                static_cast<int>(length),
                                true,
                                0);
}

/**
 * @brief Hardware alarm ISR that re-enables reconnection attempts after the cooldown window.
 */
int64_t __time_critical_func(MqttExtension::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<MqttExtension *>(user_data)->tick = true;
    return 0LL;
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
    const std::string_view _topic{topic};
    if (_topic.size() <= topicPrefix.size() + suffixLength)
    {
        return;
    }
    JsonDocument doc{};
    if (deserializeJson(doc, bytes, static_cast<size_t>(length)) == DeserializationError::Code::Ok)
    {
        DeviceService::receive(doc.as<JsonObjectConst>(),
                               name,
                               _topic.substr(topicPrefix.size(), _topic.size() - topicPrefix.size() - suffixLength));
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
    if constexpr (!Kortlinje::mqtt::host.empty())
    {
        std::array<char, 0b1U << 6U> topic{};
        if (!client.connected() || source.size() >= topic.size() - topicPrefix.size())
        {
            return;
        }
        std::memcpy(topic.data(), topicPrefix.data(), topicPrefix.size());
        std::memcpy(topic.data() + topicPrefix.size(), source.data(), source.size());
        topic[topicPrefix.size() + source.size()] = '\0';
        std::array<char, 0b1U << 10U> message{};
        const size_t length{serializeJson(payload, message.data(), message.size())};
        if (length < message.size())
        {
            client.publish(
                topic.data(), message.data(), static_cast<int>(length), false, payload["event"].isUnbound() ? 0 : 2);
        }
    }
}
