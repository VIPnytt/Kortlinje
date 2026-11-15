#include "services/FontsService.h"

#include "services/DeviceService.h"

void FontsService::begin() { transmit(); }

std::unique_ptr<const FontModule> FontsService::get(std::string_view fontName)
{

    if (fontName == KortlinjeFont::name)
    {
        return std::make_unique<const KortlinjeFont>();
    }
    if (fontName == LargeFont::name)
    {
        return std::make_unique<const LargeFont>();
    }
    if (fontName == MediumBoldFont::name)
    {
        return std::make_unique<const MediumBoldFont>();
    }
    if (fontName == MediumFont::name)
    {
        return std::make_unique<const MediumFont>();
    }
    if (fontName == MediumWideFont::name)
    {
        return std::make_unique<const MediumWideFont>();
    }
    if (fontName == MicroFont::name)
    {
        return std::make_unique<const MicroFont>();
    }
    if (fontName == MiniFont::name)
    {
        return std::make_unique<const MiniFont>();
    }
    if (fontName == SmallFont::name)
    {
        return std::make_unique<const SmallFont>();
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
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
