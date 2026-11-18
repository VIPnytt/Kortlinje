#include "extensions/MirrorExtension.h"

#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <array>
#include <span>

/**
 * @brief Broadcasts the initial inactive state on boot.
 */
void MirrorExtension::begin() { transmit(); }

/**
 * @brief Packs and broadcasts the 1-bit display frame over DDP when dirty.
 */
void MirrorExtension::handle()
{
    if (enabled && render && tick)
    {
        tick = false;
        std::array<uint8_t, 10U + Display::grid::bytes> ddp{
            (0b1U << 6U) | 0b1U,
            0U,
            (0b100U << 3U) | 0b1U,
            1U,
            0U,
            0U,
            0U,
            0U,
            static_cast<uint8_t>((Display::grid::bytes >> 8U) & 0xFFU),
            static_cast<uint8_t>(Display::grid::bytes & 0xFFU),
        };
        const auto frame{DisplayService::getFrame()};
        render = false;
        for (size_t idx{0U}; idx < frame.size(); ++idx)
        {
            if (frame[idx])
            {
                ddp[10U + (idx >> 3U)] |= static_cast<uint8_t>(0b1U << (idx & 7U));
            }
        }
        if (udp.write(ddp.data(), ddp.size()) != ddp.size())
        {
            ++interval;
            render = true;
        }
        alarmId = add_alarm_in_us(interval, &onAlarm, this, true);
    }
}

bool MirrorExtension::getEnabled() const { return enabled; }

/**
 * @brief Enables or disables UDP broadcast mirroring on DDP port 4048.
 *
 * @param enable True to start mirroring, false to stop.
 */
void MirrorExtension::setEnabled(bool enable)
{
    if (enable == enabled)
    {
        return;
    }
    if (enable && udp.connect(&addr, 4'048U))
    {
        enabled = true;
        render = true;
        tick = true;
    }
    else
    {
        const alarm_id_t _alarmId{alarmId};
        if (_alarmId > 0L)
        {
            alarmId = 0L;
            cancel_alarm(_alarmId);
        }
        udp.close();
        enabled = false;
        tick = false;
    }
    transmit();
}

int64_t __time_critical_func(MirrorExtension::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<MirrorExtension *>(user_data)};
    self->tick = true;
    self->alarmId = 0L;
    return 0LL;
}

void MirrorExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    if (payload["enabled"].is<bool>())
    {
        setEnabled(payload["enabled"].as<bool>());
    }
}

void MirrorExtension::onTransmit(JsonObjectConst payload, std::string_view source)
{
    if (enabled && source == DisplayService::name && payload["power"].is<bool>() && !payload["power"].as<bool>())
    {
        render = false;
        constexpr std::array<uint8_t, 10U + Display::grid::bytes> ddp{
            (0b1U << 6U) | 0b1U,
            0U,
            (0b100U << 3U) | 0b1U,
            1U,
            0U,
            0U,
            0U,
            0U,
            static_cast<uint8_t>((Display::grid::bytes >> 8U) & 0xFFU),
            static_cast<uint8_t>(Display::grid::bytes & 0xFFU),
        };
        udp.write(ddp.data(), ddp.size());
    }
}

void MirrorExtension::transmit()
{
    JsonDocument doc{};
    doc["enabled"].set(enabled);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
