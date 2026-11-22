#include "modes/AnimationMode.h"

#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"

#include <algorithm>
#include <string>

AnimationMode::~AnimationMode() { stopTimer(); }

void AnimationMode::begin()
{
    ConfigHandler::get(name, "interval", interval);
    uint8_t count{0U};
    if (ConfigHandler::get(name, "frames", count))
    {
        frames.resize(count);
        for (uint8_t idx{0U}; idx < count; ++idx)
        {
            if (!ConfigHandler::get(name, std::to_string(idx), frames[idx]))
            {
                frames.resize(idx);
                break;
            }
        }
    }
    if (!frames.empty())
    {
        startTimer();
    }
}

void AnimationMode::handle()
{
    if (tick && !frames.empty())
    {
        tick = false;
        if (index >= frames.size())
        {
            index = 0U;
            pending = false;
        }
        const auto &frame{frames[index]};
        DisplayService::setFrame(frame);
        if (pending)
        {
            transmit(index, frame);
        }
        ++index;
    }
}

void AnimationMode::setFrame(uint8_t _index, std::span<const uint8_t> frame)
{
    stopTimer();
    if (_index >= frames.size())
    {
        frames.resize(_index + 1U);
        ConfigHandler::set(name, "frames", static_cast<uint8_t>(frames.size()));
    }
    std::ranges::copy(frame, frames[_index].begin());
    ConfigHandler::set(name, std::to_string(_index), frames[_index]);
    index = 0U;
    pending = true;
    startTimer();
}

void AnimationMode::setFrames(uint8_t count)
{
    frames.resize(count);
    ConfigHandler::set(name, "frames", count);
    if (frames.empty())
    {
        stopTimer();
    }
    else
    {
        index = 0U;
        pending = true;
        startTimer();
    }
}

void AnimationMode::setInterval(uint32_t milliseconds)
{
    const uint32_t _interval{milliseconds * 1'000U};
    if (_interval != interval && _interval != 0U)
    {
        interval = _interval * 1'000U;
        ConfigHandler::set(name, "interval", interval);
        if (timerRunning)
        {
            stopTimer();
            startTimer();
        }
    }
}

void AnimationMode::startTimer()
{
    if (!timerRunning && !frames.empty())
    {
        timerRunning = add_repeating_timer_us(-static_cast<int64_t>(interval), &onTimer, this, &timer);
    }
}

void AnimationMode::stopTimer()
{
    if (timerRunning)
    {
        cancel_repeating_timer(&timer);
        timerRunning = false;
        tick = false;
    }
}

bool __time_critical_func(AnimationMode::onTimer)(repeating_timer *rt)
{
    static_cast<AnimationMode *>(rt->user_data)->tick = true;
    return true;
}

void AnimationMode::transmit(uint8_t _index, std::span<const uint8_t> frame)
{
    JsonDocument doc{};
    JsonArray _frame{doc["frame"].to<JsonArray>()};
    for (const uint8_t byte : frame)
    {
        _frame.add(byte);
    }
    doc["index"].set(_index);
    doc["interval"].set(interval / 1'000U);
    DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
}

void AnimationMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["action"].is<std::string_view>() && payload["action"].as<std::string_view>() == "pull")
    {
        index = 0U;
        pending = true;
    }
    if (payload["frame"].is<JsonArrayConst>() && payload["frame"].size() == Display::grid::bytes &&
        payload["index"].is<uint8_t>())
    {
        std::array<uint8_t, Display::grid::bytes> frame{};
        const JsonArrayConst _frame{payload["frame"].as<JsonArrayConst>()};
        for (size_t idx{0U}; idx < frame.size(); ++idx)
        {
            if (_frame[idx].is<uint8_t>())
            {
                frame[idx] = _frame[idx].as<uint8_t>();
            }
        }
        if constexpr (Display::grid::pixels % 8U != 0U)
        {
            frame.back() &= static_cast<uint8_t>((1U << (Display::grid::pixels % 8U)) - 1U);
        }
        setFrame(payload["index"].as<uint8_t>(), frame);
    }
    if (payload["frames"].is<uint8_t>())
    {
        setFrames(payload["frames"].as<uint8_t>());
    }
    if (payload["interval"].is<uint32_t>())
    {
        setInterval(payload["interval"].as<uint32_t>());
    }
}
