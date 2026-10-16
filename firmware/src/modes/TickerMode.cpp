#include "modes/TickerMode.h"

#include "fonts/SmallFont.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"
#include "services/ExtensionsService.h"
#include "services/FontsService.h"

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
        width = text->getWidth();
        transmit();
        pending = false;
    }
    else if (text && millis() - lastMillis > INT8_MAX)
    {
        if (width + offsetX < 0)
        {
            offsetX = GRID_COLUMNS;
        }
        lastMillis = millis();
        Display.fillFrame(false);
        text->draw(offsetX, offsetY);
        --offsetX;
    }
}

void TickerMode::end() { text.reset(); }

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

void TickerMode::transmit()
{
    JsonDocument doc{};
    doc["font"].set(font->name);
    doc["message"].set(message);
    Device.transmit(doc.as<JsonObjectConst>(), name);
}
