#pragma once

#include "modules/HandlerModule.h"
#include "services/DisplayService.h"

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>

template <std::unsigned_integral T> class BitmapHandler final : public HandlerModule
{
private:
    std::span<const T> bitmap{};

    uint8_t height{0U};
    uint8_t width{0U};

    [[nodiscard]] static constexpr uint8_t calcWidth(std::span<const T> data) noexcept
    {
        T combined{0U};
        for (const T row : data)
        {
            combined |= row;
        }
        return static_cast<uint8_t>(std::bit_width(combined));
    }

    template <std::unsigned_integral U> [[nodiscard]] static constexpr U subSat(U a, U b) noexcept
    {
        return a > b ? static_cast<U>(a - b) : U{0U};
    }

public:
    explicit BitmapHandler(std::span<const T> bitmap) noexcept;

    void draw() const;
    void draw(uint8_t x) const;
    void draw(uint8_t x, uint8_t y) const;

    [[nodiscard]] constexpr bool empty() const noexcept;

    [[nodiscard]] constexpr uint8_t getHeight() const noexcept;
    [[nodiscard]] constexpr uint8_t getWidth() const noexcept;
};

template <std::unsigned_integral T, size_t Extent>
BitmapHandler(std::span<T, Extent>) -> BitmapHandler<std::remove_cv_t<T>>;

template <std::unsigned_integral T, size_t N> BitmapHandler(const std::array<T, N> &) -> BitmapHandler<T>;

template <std::unsigned_integral T, size_t N> BitmapHandler(const T (&)[N]) -> BitmapHandler<std::remove_cv_t<T>>;

template <std::unsigned_integral T>
BitmapHandler<T>::BitmapHandler(std::span<const T> bitmap) noexcept
    : bitmap(bitmap), height(std::saturating_cast<uint8_t>(bitmap.size())), width(calcWidth(bitmap))
{
}

template <std::unsigned_integral T> void BitmapHandler<T>::draw() const
{
    draw(static_cast<uint8_t>(subSat<uint8_t>(static_cast<uint8_t>(Display::grid::columns), width) >> 1U),
         static_cast<uint8_t>(subSat<uint8_t>(static_cast<uint8_t>(Display::grid::rows), height) >> 1U));
}

template <std::unsigned_integral T> void BitmapHandler<T>::draw(uint8_t x) const
{
    draw(x, static_cast<uint8_t>(subSat<uint8_t>(static_cast<uint8_t>(Display::grid::rows), height) >> 1U));
}

template <std::unsigned_integral T> void BitmapHandler<T>::draw(uint8_t x, uint8_t y) const
{
    if (empty())
    {
        return;
    }
    const uint8_t msbMax{static_cast<uint8_t>(width - 1U)};
    const uint8_t maxCols{static_cast<uint8_t>(Display::grid::columns - x)};
    for (uint8_t row{0U}; row < static_cast<uint8_t>(std::min<size_t>(height, Display::grid::rows - y)); ++row)
    {
        T bits{bitmap[row]};
        while (bits != 0U)
        {
            const uint8_t bitPos{static_cast<uint8_t>(std::countr_zero(bits))};
            const uint8_t col{static_cast<uint8_t>(msbMax - bitPos)};
            if (col < maxCols)
            {
                DisplayService::setPixel(static_cast<uint8_t>(x + col), static_cast<uint8_t>(y + row), true);
            }
            bits = static_cast<T>(bits & static_cast<T>(bits - 1U));
        }
    }
}

template <std::unsigned_integral T> constexpr bool BitmapHandler<T>::empty() const noexcept
{
    return width == 0U || height == 0U;
}

template <std::unsigned_integral T> constexpr uint8_t BitmapHandler<T>::getHeight() const noexcept { return height; }

template <std::unsigned_integral T> constexpr uint8_t BitmapHandler<T>::getWidth() const noexcept { return width; }
