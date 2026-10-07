#include "services/FontsService.h"

#include "services/DeviceService.h"

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

FontsService &FontsService::getInstance()
{
    static FontsService instance;
    return instance;
}

FontsService &Fonts{FontsService::getInstance()};
