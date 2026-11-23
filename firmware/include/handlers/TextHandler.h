#pragma once

#include "modules/FontModule.h"
#include "modules/HandlerModule.h"
#include "services/DisplayService.h"

#include <cstdint>
#include <inplace_vector>
#include <span>
#include <string_view>
#include <variant>
#include <vector>

class TextHandler final : public HandlerModule
{
public:
    explicit TextHandler(std::string_view text, const FontModule &font, uint8_t tracking = 1U);

    void draw() const;
    void draw(uint8_t x) const;
    void draw(uint8_t x, uint8_t y) const;
    void drawMarquee(uint16_t offsetX) const;
    void drawMarquee(uint16_t offsetX, uint8_t y) const;

    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] uint8_t getHeight() const noexcept;

    [[nodiscard]] uint16_t getWidth() const noexcept;

private:
    uint8_t height{0U};

    std::variant<std::inplace_vector<uint16_t, Display::grid::columns>, std::vector<uint16_t>> buffer{};

    void drawColumns(uint8_t x, uint8_t y, uint16_t offsetX) const;

    [[nodiscard]] std::optional<char32_t> nextCodepoint(std::string_view &sv) noexcept;

    [[nodiscard]] std::span<uint16_t> appendColumns(size_t count);

    [[nodiscard]] std::span<const uint16_t> columns() const noexcept;

    template <std::unsigned_integral T> [[nodiscard]] uint8_t calcGlyphWidth(std::span<const T> bitmap) noexcept
    {
        T combined{0U};
        for (const T row : bitmap)
        {
            combined |= row;
        }
        return std::max<uint8_t>(1U, static_cast<uint8_t>(std::bit_width(combined)));
    }

    template <std::unsigned_integral T> [[nodiscard]] static constexpr T subSat(T a, T b) noexcept
    {
        return a > b ? static_cast<T>(a - b) : T{0U};
    }
};
