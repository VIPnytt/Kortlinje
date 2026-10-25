#include "modes/TickerMode.h"

#include "fonts/SmallFont.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

TickerMode::~TickerMode() { stopTimer(); }

void TickerMode::begin()
{
    if (!font)
    {
        setFont(SmallFont::name);
    }
    pending = true;
}

void TickerMode::handle()
{
    if (pending)
    {
        text = std::make_unique<TextHandler>(message, *font);
        offsetX = GRID_COLUMNS;
        offsetY = (GRID_ROWS - text->getHeight()) / 2U;
        width = static_cast<int16_t>(text->getWidth());
        transmit();
        pending = false;
        (text && width > 0) ? startTimer() : stopTimer();
    }
    else if (tick && text)
    {
        tick = false;
        if (width + offsetX < 0)
        {
            offsetX = GRID_COLUMNS;
        }
        Display.fillFrame(false);
        text->draw(offsetX, offsetY);
        --offsetX;
    }
}

void TickerMode::end()
{
    stopTimer();
    text.reset();
}

void TickerMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    // Font
    if (payload["font"].is<std::string_view>())
    {
        setFont(payload["font"].as<std::string_view>());
    }
    //  Message
    if (payload["message"].is<std::string_view>())
    {
        setMessage(payload["message"].as<std::string_view>());
    }
}

bool __time_critical_func(TickerMode::onTimer)(repeating_timer *rt)
{
    static_cast<void>(rt);
    tick = true;
    return true;
}

void TickerMode::setFont(std::string_view fontName)
{
    if (std::ranges::find(FontsService::names, fontName) != FontsService::names.end())
    {
        font = Fonts.get(fontName);
        pending = true;
    }
}

void TickerMode::setMessage(std::string_view _message)
{
    if (_message.length())
    {
        message = _message;
        pending = true;
    }
}

void TickerMode::startTimer()
{
    if (!timerRunning)
    {
        timerRunning = add_repeating_timer_ms(-(0b1L << 7U), &onTimer, nullptr, &timer);
    }
}

void TickerMode::stopTimer()
{
    if (timerRunning)
    {
        cancel_repeating_timer(&timer);
        timerRunning = false;
        tick = false;
    }
}

void TickerMode::transmit()
{
    JsonDocument doc{};
    doc["font"].set(font->name);
    doc["message"].set(message);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
