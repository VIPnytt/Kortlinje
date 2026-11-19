#include "extensions/PhotocellExtension.h"

#include "constants/HomeAssistant.h"
#include "constants/Kortlinje.h"
#include "constants/board.h"
#include "extensions/MqttExtension.h"
#include "handlers/ConfigHandler.h"
#include "pio/photocell.pio.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <ArduinoJson.h>
#include <hardware/clocks.h>
#include <hardware/irq.h>
#include <hardware/pio.h>

void PhotocellExtension::configure()
{
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        pio_add_program_at_offset(pio0, &photocell_e9_program, PIO_INSTRUCTION_COUNT - photocell_e9_program.length);
    }
    else
    {
        pio_add_program_at_offset(pio0, &photocell_program, PIO_INSTRUCTION_COUNT - photocell_program.length);
    }
    pio_sm_claim(pio0, NUM_PIO_STATE_MACHINES - 1U);
}

void PhotocellExtension::begin()
{
    ConfigHandler::get(name, "enabled", enabled);
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        pio_sm_config config{
            photocell_e9_program_get_default_config(PIO_INSTRUCTION_COUNT - photocell_e9_program.length)};
        sm_config_set_set_pins(&config, Kortlinje::pin::light_sense, 1U);
        sm_config_set_in_pins(&config, Kortlinje::pin::light_sense);
        sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU) / (0b1UL << 21U), 0U);
        sm_config_set_in_shift(&config, false, false, 0U);
        pio_gpio_init(pio0, Kortlinje::pin::light_sense);
        gpio_disable_pulls(Kortlinje::pin::light_sense);
        gpio_set_input_enabled(Kortlinje::pin::light_sense, true);
        gpio_set_drive_strength(Kortlinje::pin::light_sense, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
        pio_sm_set_consecutive_pindirs(pio0, NUM_PIO_STATE_MACHINES - 1U, Kortlinje::pin::light_sense, 1U, false);
        pio_sm_init(pio0, NUM_PIO_STATE_MACHINES - 1U, PIO_INSTRUCTION_COUNT - photocell_e9_program.length, &config);
    }
    else
    {
        pio_sm_config config{photocell_program_get_default_config(PIO_INSTRUCTION_COUNT - photocell_program.length)};
        sm_config_set_in_pins(&config, Kortlinje::pin::light_sense);
        sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU) / (0b1UL << 21U), 0U);
        sm_config_set_in_shift(&config, false, false, 0U);
        pio_gpio_init(pio0, Kortlinje::pin::light_sense);
        gpio_disable_pulls(Kortlinje::pin::light_sense);
        gpio_set_input_enabled(Kortlinje::pin::light_sense, true);
        pio_sm_init(pio0, NUM_PIO_STATE_MACHINES - 1U, PIO_INSTRUCTION_COUNT - photocell_program.length, &config);
    }
    pio_sm_clear_fifos(pio0, NUM_PIO_STATE_MACHINES - 1U);
    pio_interrupt_clear(pio0, 1U);
    pio_set_irq1_source_enabled(pio0, pio_interrupt_source::pis_interrupt1, true);
    irq_set_exclusive_handler(irq_num_rp2350::PIO0_IRQ_1, &onPio);
    irq_set_enabled(irq_num_rp2350::PIO0_IRQ_1, true);
    if (enabled)
    {
        pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, true);
    }
}

void PhotocellExtension::handle()
{
    if (pending)
    {
        pending = false;
        if (enabled)
        {
            DisplayService::setPower(lit);
        }
        transmit();
    }
}

void PhotocellExtension::cancelAlarm()
{
    const alarm_id_t _alarmId{alarmId};
    if (_alarmId > 0L)
    {
        alarmId = 0L;
        cancel_alarm(_alarmId);
    }
}

bool PhotocellExtension::getDetected() const { return lit; }

bool PhotocellExtension::getEnabled() const { return enabled; }

int64_t __time_critical_func(PhotocellExtension::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<void>(user_data);
    alarmId = 0L;
    lit = false;
    pending = true;
    return 0LL;
}

/**
 * @brief Adds Home Assistant discovery entries for photocell activation and illuminance.
 *
 * @param discovery JSON document receiving the discovery configuration.
 * @param topic Base topic for the generated entities.
 * @param unique Prefix used to generate unique entity identifiers.
 */
void PhotocellExtension::onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique)
{
    topic.append(name);
    {
        const std::string id{std::string(name).append("_enabled")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::command_template].set(R"({"enabled":{{value}}})");
        component[HomeAssistant::Abbreviations::command_topic].set(topic + "/set");
        component[HomeAssistant::Abbreviations::icon].set("mdi:brightness-auto");
        component[HomeAssistant::Abbreviations::name].set(name);
        component[HomeAssistant::Abbreviations::payload_off].set("false");
        component[HomeAssistant::Abbreviations::payload_on].set("true");
        component[HomeAssistant::Abbreviations::platform].set("switch");
        component[HomeAssistant::Abbreviations::state_off].set("False");
        component[HomeAssistant::Abbreviations::state_on].set("True");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.enabled}}");
    }
    {
        const std::string id{std::string(name).append("_lit")};
        JsonObject component{discovery[HomeAssistant::Abbreviations::components][id].to<JsonObject>()};
        component[HomeAssistant::Abbreviations::device_class].set("light");
        component[HomeAssistant::Abbreviations::enabled_by_default].set(false);
        component[HomeAssistant::Abbreviations::icon].set("mdi:brightness-5");
        component[HomeAssistant::Abbreviations::name].set(name);
        component[HomeAssistant::Abbreviations::payload_off].set("False");
        component[HomeAssistant::Abbreviations::payload_on].set("True");
        component[HomeAssistant::Abbreviations::platform].set("binary_sensor");
        component[HomeAssistant::Abbreviations::state_topic].set(topic);
        component[HomeAssistant::Abbreviations::unique_id].set(unique + id);
        component[HomeAssistant::Abbreviations::value_template].set("{{value_json.lit}}");
    }
}

void __time_critical_func(PhotocellExtension::onPio)()
{
    pio_interrupt_clear(pio0, 1U);
    while (!pio_sm_is_rx_fifo_empty(pio0, NUM_PIO_STATE_MACHINES - 1U))
    {
        const bool light{pio_sm_get(pio0, NUM_PIO_STATE_MACHINES - 1U) != 0U};
        if (light)
        {
            cancelAlarm();
            if (!lit)
            {
                lit = true;
                pending = true;
            }
        }
        else if (lit && alarmId <= 0L)
        {
            alarmId = add_alarm_in_us(0b1ULL << 25U, &onAlarm, nullptr, true);
        }
    }
}

void PhotocellExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["enabled"].is<bool>())
    {
        setEnabled(payload["enabled"].as<bool>());
    }
}

void PhotocellExtension::setEnabled(bool enable)
{
    if (enable == enabled)
    {
        return;
    }
    enabled = enable;
    cancelAlarm();
    lit = false;
    pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, false);
    pio_sm_clear_fifos(pio0, NUM_PIO_STATE_MACHINES - 1U);
    pio_interrupt_clear(pio0, 1U);
    if constexpr (Kortlinje::board::type == "RP2350" && Kortlinje::board::revision <= 0xA2U)
    {
        if (enabled)
        {
            pio_sm_exec(
                pio0, NUM_PIO_STATE_MACHINES - 1U, pio_encode_jmp(PIO_INSTRUCTION_COUNT - photocell_e9_program.length));
            pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, true);
        }
        else
        {
            pio_sm_set_consecutive_pindirs(pio0, NUM_PIO_STATE_MACHINES - 1U, Kortlinje::pin::light_sense, 1U, false);
        }
    }
    else
    {
        if (enabled)
        {
            pio_sm_exec(
                pio0, NUM_PIO_STATE_MACHINES - 1U, pio_encode_jmp(PIO_INSTRUCTION_COUNT - photocell_program.length));
            pio_sm_set_enabled(pio0, NUM_PIO_STATE_MACHINES - 1U, true);
        }
    }
    ConfigHandler::set(name, "enabled", enabled);
    pending = true;
}

void PhotocellExtension::transmit()
{
    JsonDocument doc{};
    doc["enabled"].set(enabled);
    doc["lit"].set(lit);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
