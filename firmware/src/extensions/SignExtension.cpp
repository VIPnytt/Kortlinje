#include "extensions/SignExtension.h"

#include "handlers/BitmapHandler.h"
#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ModesService.h"

#include <algorithm>
#include <span>

/**
 * @brief Loads the stored signal duration and transmits the current configuration.
 */
void SignExtension::begin()
{
    ConfigHandler::get(name, "duration", duration);
    transmit();
}

/**
 * @brief Displays a pending signal bitmap or restores the previous frame once expired.
 */
void SignExtension::handle()
{
    if (pending && DisplayService::getPower())
    {
        pending = false;
        tick = false;
        if (!enabled)
        {
            ModesService::setActive(false);
            std::ranges::copy(DisplayService::getFrame(), frame.begin());
            enabled = true;
        }
        DisplayService::fillFrame(false);
        BitmapHandler(std::span<const uint64_t>{bitmap}).draw();
        if (alarmId > 0)
        {
            cancel_alarm(alarmId);
        }
        alarmId = add_alarm_in_ms(duration, &SignExtension::onAlarm, this, true);
        JsonDocument doc{};
        doc["event"].set("sign");
        DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
    }
    else if (tick && enabled && DisplayService::getPower())
    {
        tick = false;
        DisplayService::setFrame(frame);
        ModesService::setActive(true);
        enabled = false;
    }
}

int64_t SignExtension::onAlarm(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<SignExtension *>(user_data)};
    self->alarmId = 0L;
    self->tick = true;
    return 0LL;
}

/**
 * @brief Applies an incoming signal duration and stores up to GRID_ROWS 64-bit row bitmasks.
 *
 * @param payload Incoming duration and bitmap configuration.
 */
void SignExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["duration"].is<uint8_t>())
    {
        setDuration(payload["duration"].as<uint8_t>());
    }
    if (payload["bitmap"].is<JsonArrayConst>())
    {
        const JsonArrayConst _bitmap{payload["bitmap"].as<JsonArrayConst>()};
        if (!_bitmap.isNull() && _bitmap.size() > 0U && _bitmap.size() <= GRID_ROWS)
        {
            bitmap.fill(0U);
            size_t row{(GRID_ROWS - _bitmap.size()) / 2U};
            for (const JsonVariantConst value : _bitmap)
            {
                if (value.is<uint64_t>())
                {
                    bitmap[row] = value.as<uint64_t>();
                }
                ++row;
            }
            pending = true;
        }
    }
}

void SignExtension::setDuration(uint8_t seconds)
{
    const uint32_t _duration{seconds * 1'000U};
    if (_duration != duration && _duration != 0U)
    {
        duration = _duration;
        ConfigHandler::set(name, "duration", duration);
        transmit();
    }
}

/**
 * @brief Transmits the configured signal duration in seconds.
 */
void SignExtension::transmit()
{
    JsonDocument doc{};
    doc["duration"].set(duration / 1'000U);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
