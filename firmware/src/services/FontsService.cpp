#include "services/FontsService.h"

#include "fonts/KortlinjeFont.h"
#include "fonts/LargeFont.h"
#include "fonts/MediumBoldFont.h"
#include "fonts/MediumFont.h"
#include "fonts/MediumWideFont.h"
#include "fonts/MicroFont.h"
#include "fonts/MiniFont.h"
#include "fonts/SmallFont.h"
#include "services/DeviceService.h"

#include <array>

void FontsService::begin() { transmit(); }

std::unique_ptr<const FontModule> FontsService::getFont(std::string_view fontName)
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

std::span<const std::string_view> FontsService::getNames()
{
    static constexpr std::array names{
        MicroFont::name,
        MiniFont::name,
        SmallFont::name,
        MediumFont::name,
        MediumBoldFont::name,
        MediumWideFont::name,
        KortlinjeFont::name,
        LargeFont::name,
    };
    return names;
}

void FontsService::transmit()
{
    JsonDocument doc{};
    JsonArray list{doc["list"].to<JsonArray>()};
    for (const std::string_view _name : getNames())
    {
        list.add(_name);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
