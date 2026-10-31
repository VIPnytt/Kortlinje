#include "extensions/AlarmExtension.h"

#include "config/secrets.h"
#include "handlers/ConfigHandler.h"
#include "pio/alarm.pio.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

void AlarmExtension::configure()
{
    pio_add_program_at_offset(pio2, &alarm_program, 0U);
    pio_sm_claim(pio2, 0U);
}

void AlarmExtension::begin()
{
    pio_sm_config config{alarm_program_get_default_config(0U)};
    sm_config_set_sideset_pins(&config, PIN_BUZ);
    sm_config_set_set_pins(&config, PIN_BUZ, 1U);
    sm_config_set_clkdiv_int_frac8(&config, static_cast<uint32_t>(F_CPU) / (0b1UL << 13U), 0U);
    pio_gpio_init(pio2, PIN_BUZ);
    gpio_disable_pulls(PIN_BUZ);
    gpio_set_drive_strength(PIN_BUZ, gpio_drive_strength::GPIO_DRIVE_STRENGTH_2MA);
    pio_sm_set_consecutive_pindirs(pio2, 0U, PIN_BUZ, 1U, true);
    pio_sm_init(pio2, 0U, 0U, &config);
    pio_sm_clear_fifos(pio2, 0U);
    pio_sm_put_blocking(pio2, 0U, 0b1UL << 12U);
    pio_sm_exec(pio2, 0U, pio_encode_pull(false, false));
    pio_sm_exec(pio2, 0U, pio_encode_mov(pio_src_dest::pio_isr, pio_src_dest::pio_osr));
    pio_sm_put_blocking(pio2, 0U, 0b1UL << 8U);
    pio_sm_exec(pio2, 0U, pio_encode_pull(false, false));
    pio_sm_exec(pio2, 0U, pio_encode_set(pio_src_dest::pio_pins, 0U));
    std::array<uint8_t, 3U> data{};
    if (ConfigHandler::get(name, "alarms", data))
    {
        const uint32_t blob{static_cast<uint32_t>(data[0U]) | (static_cast<uint32_t>(data[1U]) << 8U) |
                            (static_cast<uint32_t>(data[2U]) << 16U)};
        alarm1.hour = static_cast<uint8_t>((blob >> 6U) & 0x1FUL);
        alarm2.hour = static_cast<uint8_t>((blob >> 18U) & 0x1FUL);
        alarm1.minute = static_cast<uint8_t>(blob & 0x3FUL);
        alarm2.minute = static_cast<uint8_t>((blob >> 12U) & 0x3FUL);
        alarm1.enabled = ((blob >> 11U) & 0x01UL) != 0UL;
        alarm2.enabled = ((blob >> 23U) & 0x01UL) != 0UL;
        if (alarm1.enabled || alarm2.enabled)
        {
            reschedule();
            Display.setPixel(DisplayService::Symbol::ALARM1, alarm1.enabled);
            Display.setPixel(DisplayService::Symbol::ALARM2, alarm2.enabled);
        }
    }
}

void AlarmExtension::handle()
{
    if (pending)
    {
        pending = false;
        transmit();
    }
    else if (timeout)
    {
        timeout = false;
        dismiss();
    }
}

void AlarmExtension::save()
{
    const uint32_t blob{
        static_cast<uint32_t>(alarm1.minute & 0x3FUL) | (static_cast<uint32_t>(alarm1.hour & 0x1FUL) << 6U) |
        static_cast<uint32_t>(alarm1.enabled ? 0b1UL << 11U : 0UL) |
        (static_cast<uint32_t>(alarm2.minute & 0x3FUL) << 12U) | (static_cast<uint32_t>(alarm2.hour & 0x1FUL) << 18U) |
        static_cast<uint32_t>(alarm2.enabled ? 0b1UL << 23U : 0U)};
    const std::array<uint8_t, 3U> data{static_cast<uint8_t>(blob & 0xFFUL),
                                       static_cast<uint8_t>((blob >> 8U) & 0xFFUL),
                                       static_cast<uint8_t>((blob >> 16U) & 0xFFUL)};
    ConfigHandler::set(name, "alarms", data);
}

void AlarmExtension::cancelTimer()
{
    if (alarmId > 0L)
    {
        cancel_alarm(alarmId);
        alarmId = 0L;
    }
}

void AlarmExtension::dismiss()
{
    cancelTimer();
    pio_sm_set_enabled(pio2, 0U, false);
    pio_sm_exec(pio2, 0U, pio_encode_set(pio_src_dest::pio_pins, 0U));
    state = State::IDLE;
    reschedule();
    Display.setPixel(DisplayService::Symbol::SNOOZE, false);
}

bool AlarmExtension::getAlarm1() { return alarm1.enabled; }

bool AlarmExtension::getAlarm2() { return alarm2.enabled; }

time_t AlarmExtension::getEpoch(time_t now, AlarmTime config)
{
    struct tm timeInfo{};
    localtime_r(&now, &timeInfo);
    timeInfo.tm_hour = static_cast<int>(config.hour);
    timeInfo.tm_min = static_cast<int>(config.minute);
    timeInfo.tm_sec = 0;
    timeInfo.tm_isdst = -1;
    time_t target{mktime(&timeInfo)};
    if (target <= now)
    {
        timeInfo.tm_mday += 1;
        timeInfo.tm_isdst = -1;
        target = mktime(&timeInfo);
    }
    return target;
}

void AlarmExtension::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["action"].is<std::string_view>())
    {
        const std::string_view action{payload["action"].as<std::string_view>()};
        if (action == "dismiss")
        {
            dismiss();
        }
        else if (action == "snooze")
        {
            snooze();
        }
    }
    if (payload["alarm1"]["enabled"].is<bool>())
    {
        setAlarm1(payload["alarm1"]["enabled"].as<bool>());
    }
    if (payload["alarm2"]["enabled"].is<bool>())
    {
        setAlarm2(payload["alarm2"]["enabled"].as<bool>());
    }
    if (payload["alarm1"]["hour"].is<uint8_t>() && payload["alarm1"]["minute"].is<uint8_t>())
    {
        setTime1(payload["alarm1"]["hour"].as<uint8_t>(), payload["alarm1"]["minute"].as<uint8_t>());
    }
    if (payload["alarm2"]["hour"].is<uint8_t>() && payload["alarm2"]["minute"].is<uint8_t>())
    {
        setTime2(payload["alarm2"]["hour"].as<uint8_t>(), payload["alarm2"]["minute"].as<uint8_t>());
    }
}

int64_t __time_critical_func(AlarmExtension::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    static_cast<void>(user_data);
    alarmId = 0L;
    if (state == State::RINGING)
    {
        pio_sm_set_enabled(pio2, 0U, false);
        pio_sm_exec(pio2, 0U, pio_encode_set(pio_src_dest::pio_pins, 0U));
        timeout = true;
    }
    else
    {
        state = State::RINGING;
        pio_sm_clkdiv_restart(pio2, 0U);
        pio_sm_exec(pio2, 0U, pio_encode_jmp(0U));
        pio_sm_set_enabled(pio2, 0U, true);
        Display.setPixel(DisplayService::Symbol::SNOOZE, false);
        Display.setPower(true);
        alarmId = add_alarm_in_ms(0b1UL << 20U, &onAlarm, nullptr, true);
    }
    return 0LL;
}

void AlarmExtension::reschedule()
{
    if (state != State::IDLE)
    {
        return;
    }
    cancelTimer();
    if (!alarm1.enabled && !alarm2.enabled)
    {
        return;
    }
    const time_t now{time(nullptr)};
    const time_t _epoch1{alarm1.enabled ? getEpoch(now, alarm1) : 0LL};
    const time_t _epoch2{alarm2.enabled ? getEpoch(now, alarm2) : 0LL};
    time_t epoch{0LL};
    if (_epoch1 > 0LL && _epoch2 > 0LL)
    {
        epoch = min(_epoch1, _epoch2);
    }
    else if (_epoch1 > 0LL)
    {
        epoch = _epoch1;
    }
    else if (_epoch2 > 0LL)
    {
        epoch = _epoch2;
    }
    if (epoch > now)
    {
        alarmId = add_alarm_in_ms(
            static_cast<uint32_t>(static_cast<uint64_t>(epoch - now) * 1'000ULL), &onAlarm, nullptr, true);
    }
}

void AlarmExtension::setAlarm1(bool enable)
{
    if (alarm1.enabled == enable)
    {
        return;
    }
    alarm1.enabled = enable;
    reschedule();
    Display.setPixel(DisplayService::Symbol::ALARM1, alarm1.enabled);
    save();
    pending = true;
}

void AlarmExtension::setAlarm2(bool enable)
{
    if (alarm2.enabled == enable)
    {
        return;
    }
    alarm2.enabled = enable;
    reschedule();
    Display.setPixel(DisplayService::Symbol::ALARM2, alarm2.enabled);
    save();
    pending = true;
}

void AlarmExtension::setTime1(uint8_t hour, uint8_t minute)
{
    if (alarm1.hour == hour && alarm1.minute == minute)
    {
        return;
    }
    alarm1.hour = hour;
    alarm1.minute = minute;
    reschedule();
    save();
    pending = true;
}

void AlarmExtension::setTime2(uint8_t hour, uint8_t minute)
{
    if (alarm2.hour == hour && alarm2.minute == minute)
    {
        return;
    }
    alarm2.hour = hour;
    alarm2.minute = minute;
    reschedule();
    save();
    pending = true;
}

void AlarmExtension::snooze()
{
    if (state != State::RINGING)
    {
        return;
    }
    cancelTimer();
    pio_sm_set_enabled(pio2, 0U, false);
    pio_sm_exec(pio2, 0U, pio_encode_set(pio_src_dest::pio_pins, 0U));
    alarmId = add_alarm_in_ms(0b1UL << 19U, &onAlarm, nullptr, true);
    state = State::SNOOZED;
    Display.setPixel(DisplayService::Symbol::SNOOZE, true);
}

void AlarmExtension::transmit()
{
    JsonDocument doc{};
    doc["alarm1"]["enabled"].set(alarm1.enabled);
    doc["alarm1"]["hour"].set(alarm1.hour);
    doc["alarm1"]["minute"].set(alarm1.minute);
    doc["alarm2"]["enabled"].set(alarm2.enabled);
    doc["alarm2"]["hour"].set(alarm2.hour);
    doc["alarm2"]["minute"].set(alarm2.minute);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
