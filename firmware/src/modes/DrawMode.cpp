#include "modes/DrawMode.h"

#include "handlers/ConfigHandler.h"
#include "services/DeviceService.h"

#include <algorithm>

DrawMode::~DrawMode() { save(true); }

void DrawMode::begin()
{
    load(true);
    if (!render)
    {
        for (const uint8_t byte : frame)
        {
            if (byte != 0U)
            {
                pending = true;
                render = true;
                break;
            }
        }
    }
}

void DrawMode::handle()
{
    if (render)
    {
        DisplayService::setFrame(frame);
        render = false;
    }
    else if (pending)
    {
        transmit();
        pending = false;
    }
}

void DrawMode::load(bool cache)
{
    if ((cache && ConfigHandler::get(name, "cache", frame)) || ConfigHandler::get(name, "saved", frame))
    {
        pending = true;
        render = true;
    }
}

void DrawMode::save(bool cache)
{
    if (std::ranges::any_of(frame, [](uint8_t byte) { return byte != 0U; }))
    {
        ConfigHandler::set(name, cache ? "cache" : "saved", frame);
        pending = true;
    }
}

void DrawMode::transmit()
{
    JsonDocument doc{};
    JsonArray _frame{doc["frame"].to<JsonArray>()};
    for (const uint8_t byte : frame)
    {
        _frame.add(byte);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name, false);
}

void DrawMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    // Action
    if (payload["action"].is<std::string_view>())
    {
        const std::string_view action{payload["action"].as<std::string_view>()};
        if (action == "clear")
        {
            frame.fill(0U);
            render = true;
        }
        else if (action == "load")
        {
            load();
        }
        else if (action == "pull")
        {
            save(true);
        }
        else if (action == "save")
        {
            save();
        }
    }
    if (payload["frame"].is<JsonArrayConst>() && payload["frame"].size() == frame.size())
    {
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
        render = true;
    }
    if (payload["pixels"].is<JsonArrayConst>())
    {
        for (const JsonVariantConst &pixel : payload["pixels"].as<JsonArrayConst>())
        {
            if (pixel["x"].is<uint8_t>() && pixel["y"].is<uint8_t>() && pixel["lit"].is<bool>())
            {
                const uint8_t x{pixel["x"].as<uint8_t>()};
                const uint8_t y{pixel["y"].as<uint8_t>()};
                if (x < Display::grid::columns && y < Display::grid::rows)
                {
                    const size_t idx{static_cast<size_t>(x + (y * Display::grid::columns))};
                    if (pixel["lit"].as<bool>())
                    {
                        frame[idx / 8U] |= static_cast<uint8_t>(1U << (idx % 8U));
                    }
                    else
                    {
                        frame[idx / 8U] &= static_cast<uint8_t>(~(1U << (idx % 8U)));
                    }
                }
            }
        }
        render = true;
    }
}
