#pragma once

#include "fonts/KortlinjeFont.h"
#include "fonts/SmallFont.h"
#include "modules/ServiceModule.h"

#include <array>
#include <bits/unique_ptr.h>
#include <string_view>

class FontsService final : public ServiceModule
{
private:
    explicit FontsService() : ServiceModule("Fonts") {};

public:
    [[nodiscard]] std::unique_ptr<const FontModule> get(std::string_view fontName) const;

    static constexpr std::array<std::string_view, 2U> names{
        SmallFont::name,
        KortlinjeFont::name,
    };

    static FontsService &getInstance();
};

extern FontsService &Fonts;
