#pragma once

#include "fonts/KortlinjeFont.h"
#include "fonts/LargeFont.h"
#include "fonts/MediumBoldFont.h"
#include "fonts/MediumFont.h"
#include "fonts/MediumWideFont.h"
#include "fonts/MicroFont.h"
#include "fonts/MiniFont.h"
#include "fonts/SmallFont.h"
#include "modules/ServiceModule.h"

#include <array>
#include <bits/unique_ptr.h>
#include <string_view>

class FontsService final : public ServiceModule
{
private:
    void transmit();

public:
    explicit FontsService() : ServiceModule("Fonts") {};

    void begin();

    [[nodiscard]] static std::unique_ptr<const FontModule> get(std::string_view fontName);

    static constexpr std::array<std::string_view, 8U> names{
        MicroFont::name,
        MiniFont::name,
        SmallFont::name,
        MediumFont::name,
        MediumBoldFont::name,
        MediumWideFont::name,
        KortlinjeFont::name,
        LargeFont::name,
    };
};
