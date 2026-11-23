#include "handlers/TextHandler.h"

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <limits>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

TextHandler::TextHandler(std::string_view text, const FontModule &font, uint8_t tracking)
{
    int ascent{0};
    int yMin{0};
    while (const std::optional<char32_t> codepoint{nextCodepoint(text)})
    {
        const FontModule::Symbol character{font.getChar(*codepoint)};
        yMin = std::min<int>(yMin, character.offsetY);
        std::visit(
            [&](auto bitmap)
            {
                using RowT = typename decltype(bitmap)::value_type;
                if (!bitmap.empty())
                {
                    const uint8_t glyphHeight{static_cast<uint8_t>(bitmap.size())};
                    const int glyphTopExtent{static_cast<int>(glyphHeight) + static_cast<int>(character.offsetY)};
                    if (glyphTopExtent > ascent)
                    {
                        const int shift{glyphTopExtent - ascent};
                        std::visit(
                            [shift](auto &storage)
                            {
                                for (uint16_t &col : storage)
                                {
                                    col = shift < 16 ? static_cast<uint16_t>(col << shift) : 0U;
                                }
                            },
                            buffer);
                        ascent = glyphTopExtent;
                    }
                    const int glyphTop{ascent - glyphTopExtent};
                    const uint8_t glyphWidth{calcGlyphWidth(bitmap)};
                    const size_t gap{columns().empty() ? 0U : tracking};
                    const std::span<uint16_t> glyphCols{
                        appendColumns(gap + static_cast<size_t>(character.offsetX) + glyphWidth).last(glyphWidth)};
                    for (uint8_t row{0U}; row < glyphHeight; ++row)
                    {
                        const int destRow{glyphTop + static_cast<int>(row)};
                        if (destRow >= 16)
                        {
                            break;
                        }
                        const uint16_t rowBit{static_cast<uint16_t>(1U << destRow)};
                        RowT bits{bitmap[row]};
                        while (bits != 0U)
                        {
                            const uint8_t bit{static_cast<uint8_t>(std::countr_zero(bits))};
                            glyphCols[glyphWidth - 1U - bit] |= rowBit;
                            bits = static_cast<RowT>(bits & static_cast<RowT>(bits - 1U));
                        }
                    }
                }
                else if (character.offsetX != 0U)
                {
                    const size_t gap{columns().empty() ? 0U : tracking};
                    static_cast<void>(appendColumns(gap + static_cast<size_t>(character.offsetX)));
                }
            },
            character.bitmap);
    }
    height = std::saturating_cast<uint8_t>(ascent - yMin);
}

std::span<uint16_t> TextHandler::appendColumns(size_t count)
{
    if (auto *inlineBuf{std::get_if<std::inplace_vector<uint16_t, Display::grid::columns>>(&buffer)})
    {
        const size_t prevSize{inlineBuf->size()};
        if (prevSize + count <= inlineBuf->capacity())
        {
            inlineBuf->resize(prevSize + count, 0U);
            return std::span<uint16_t>{*inlineBuf}.subspan(prevSize, count);
        }
        std::vector<uint16_t> heapBuf{};
        heapBuf.reserve(std::max<size_t>(prevSize + count, inlineBuf->capacity() * 2U));
        heapBuf.assign(inlineBuf->begin(), inlineBuf->end());
        heapBuf.resize(prevSize + count, 0U);
        auto &stored{buffer.emplace<std::vector<uint16_t>>(std::move(heapBuf))};
        return std::span<uint16_t>{stored}.subspan(prevSize, count);
    }
    auto &heapBuf{std::get<std::vector<uint16_t>>(buffer)};
    const size_t prevSize{heapBuf.size()};
    heapBuf.resize(prevSize + count, 0U);
    return std::span<uint16_t>{heapBuf}.subspan(prevSize, count);
}

std::span<const uint16_t> TextHandler::columns() const noexcept
{
    return std::visit([](const auto &storage) noexcept -> std::span<const uint16_t> { return storage; }, buffer);
}

void TextHandler::draw() const
{
    draw(static_cast<uint8_t>(subSat<uint16_t>(Display::grid::columns, getWidth()) >> 1U),
         static_cast<uint8_t>(subSat<uint8_t>(static_cast<uint8_t>(Display::grid::rows), height) >> 1U));
}

void TextHandler::draw(uint8_t x) const
{
    drawColumns(x, static_cast<uint8_t>(subSat<uint8_t>(static_cast<uint8_t>(Display::grid::rows), height) >> 1U), 0U);
}

void TextHandler::draw(uint8_t x, uint8_t y) const { drawColumns(x, y, 0U); }

void TextHandler::drawColumns(uint8_t x, uint8_t y, uint16_t offsetX) const
{
    const std::span<const uint16_t> cols{columns()};
    if (x >= Display::grid::columns || y >= Display::grid::rows || offsetX >= cols.size())
    {
        return;
    }
    const uint8_t visibleRows{static_cast<uint8_t>(Display::grid::rows - y)};
    const uint16_t rowMask{static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() >> (16U - visibleRows))};
    for (uint8_t col{0U};
         col < static_cast<uint8_t>(std::min<size_t>(cols.size() - offsetX, Display::grid::columns - x));
         ++col)
    {
        uint16_t bits{static_cast<uint16_t>(cols[offsetX + col] & rowMask)};
        while (bits != 0U)
        {
            const uint8_t row{static_cast<uint8_t>(std::countr_zero(bits))};
            DisplayService::setPixel(static_cast<uint8_t>(x + col), static_cast<uint8_t>(y + row), true);
            bits = static_cast<uint16_t>(bits & static_cast<uint16_t>(bits - 1U));
        }
    }
}

void TextHandler::drawMarquee(uint16_t offsetX) const
{
    drawMarquee(offsetX,
                static_cast<uint8_t>(subSat<uint8_t>(static_cast<uint8_t>(Display::grid::rows), height) >> 1U));
}

void TextHandler::drawMarquee(uint16_t offsetX, uint8_t y) const
{
    const uint8_t x{offsetX < Display::grid::columns ? static_cast<uint8_t>(Display::grid::columns - offsetX)
                                                     : uint8_t{0U}};
    const uint16_t srcX{offsetX > Display::grid::columns ? static_cast<uint16_t>(offsetX - Display::grid::columns)
                                                         : uint16_t{0U}};
    drawColumns(x, y, srcX);
}

bool TextHandler::empty() const noexcept { return columns().empty(); }

uint8_t TextHandler::getHeight() const noexcept { return height; }

uint16_t TextHandler::getWidth() const noexcept { return std::saturating_cast<uint16_t>(columns().size()); }

std::optional<char32_t> TextHandler::nextCodepoint(std::string_view &sv) noexcept
{
    if (sv.empty())
    {
        return std::nullopt;
    }
    const uint8_t first{static_cast<uint8_t>(sv.front())};
    if (first < 0x80U)
    {
        sv.remove_prefix(1U);
        return static_cast<char32_t>(first);
    }
    const int len{std::countl_one(first)};
    if (len < 2 || len > 4)
    {
        sv.remove_prefix(1U);
        return U'\uFFFD';
    }
    const size_t seqLen{static_cast<size_t>(len)};
    if (sv.size() < seqLen)
    {
        sv = {};
        return U'\uFFFD';
    }
    static constexpr std::array<uint8_t, 5U> masks{0x7FU, 0U, 0x1FU, 0xFU, 0x7U};
    static constexpr std::array<char32_t, 5U> mins{0U, 0U, 0x80U, 0x800U, 0x10000U};
    char32_t codepoint{static_cast<char32_t>(first & masks[seqLen])};
    for (size_t i{1U}; i < seqLen; ++i)
    {
        const uint8_t next{static_cast<uint8_t>(sv[i])};
        if ((next & 0xC0U) != 0x80U)
        {
            sv.remove_prefix(i);
            return U'\uFFFD';
        }
        codepoint = (codepoint << 6U) | static_cast<char32_t>(next & 0x3FU);
    }
    sv.remove_prefix(seqLen);
    if (codepoint < mins[seqLen] || codepoint > 0x10FFFFU || (codepoint >= 0xD800U && codepoint <= 0xDFFFU))
    {
        return U'\uFFFD';
    }
    return codepoint;
}
