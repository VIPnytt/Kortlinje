#pragma once

#include "modules/FontModule.h"
#include "modules/ServiceModule.h"

#include <bits/unique_ptr.h>
#include <memory>
#include <span>
#include <string_view>

class FontsService final : public ServiceModule
{
private:
    void transmit();

public:
    explicit FontsService() : ServiceModule("Fonts") {};

    void begin();

    [[nodiscard]] static std::span<const std::string_view> getCollection();

    [[nodiscard]] static std::unique_ptr<const FontModule> getFont(std::string_view fontName);
};
