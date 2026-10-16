#include "services/FontsService.h"

#include "services/DeviceService.h"

void FontsService::begin() { transmit(); }

std::unique_ptr<const FontModule> FontsService::get(std::string_view fontName) const
{
    if (fontName == SmallFont::name)
    {
        return std::make_unique<const SmallFont>();
    }
    if (fontName == KortlinjeFont::name)
    {
        return std::make_unique<const KortlinjeFont>();
    }
    return nullptr;
}

void FontsService::transmit()
{
    JsonDocument doc{};
    JsonArray list{doc["list"].to<JsonArray>()};
    for (const std::string_view _name : names)
    {
        list.add(_name);
    }
    Device.transmit(doc.as<JsonObjectConst>(), name);
}

FontsService &FontsService::getInstance()
{
    static FontsService instance;
    return instance;
}

FontsService &Fonts{FontsService::getInstance()};
