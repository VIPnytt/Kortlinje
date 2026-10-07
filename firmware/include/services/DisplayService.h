#pragma once

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
#include "modules/ServiceModule.h"

#include <Arduino.h>
#include <array>
#include <span>

class DisplayService final : public ServiceModule
{
public:
    enum class Brightness : uint8_t
    {
        DUTY16_1 = 0b1000'1000U,
        DUTY16_2 = 0b1000'1001U,
        DUTY16_4 = 0b1000'1010U,
        DUTY16_10 = 0b1000'1011U,
        DUTY16_11 = 0b1000'1100U,
        DUTY16_12 = 0b1000'1101U,
        DUTY16_13 = 0b1000'1110U,
        DUTY16_14 = 0b1000'1111U,
    };

    enum class Symbol : uint8_t
    {
        AM,
        PM,
        AL1,
        SNZ,
        AL2,
        STATUS,
    };

    bool power{false};

    Brightness brightness{Brightness::DUTY16_14};

    void begin();
    void drawEllipseOutline(float x, float y, float radius, bool lit);
    void drawEllipseSolid(float x, float y, float radius, bool lit);
    void drawLineHorizontal(uint8_t xMin, size_t columns, uint8_t y, bool lit);
    void drawLineVertical(uint8_t x, uint8_t yMin, uint8_t yMax, bool lit);
    void drawRectangleOutline(size_t minX, size_t columns, size_t minY, size_t maxY, bool lit);
    void drawRectangleSolid(uint8_t minX, uint8_t columns, uint8_t minY, uint8_t maxY, bool lit);
    void fillColumn(uint8_t x, bool lit);
    void fillFrame(bool lit);
    void fillRow(uint8_t y, bool lit);
    void fillRows(size_t minY, size_t rows, bool lit);
    void flush();
    void getFrame(std::span<bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> _frame) const;
    void invertFrame();
    void setBrightness(Brightness _brightness);
    void setFrame(std::span<const bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> _frame);
    void setPixel(size_t idx, bool lit);
    void setPixel(uint8_t x, uint8_t y, bool lit);
    void setPixel(Symbol symbol, bool lit);
    void setPower(bool _power);

    [[nodiscard]] bool getPixel(size_t idx) const;
    [[nodiscard]] bool getPixel(uint8_t x, uint8_t y) const;
    [[nodiscard]] bool getPixel(Symbol symbol) const;
    [[nodiscard]] bool getPower() const;

    [[nodiscard]] Brightness getBrightness() const;

    static DisplayService &getInstance();

private:
    explicit DisplayService() : ServiceModule("Display") {};

    static constexpr uint32_t cycles{static_cast<uint32_t>(
        ((static_cast<unsigned long long>(CLK_PULSE_WIDTH) * static_cast<unsigned long long>(F_CPU)) + 999'999'999ULL) /
        1'000'000'000ULL)};

    static constexpr std::array<uint16_t, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> pixels{LED_MAP};

    bool render{false};

    std::array<bool, pixels.size()> frame{};

    std::array<pin_size_t, (pixels.size() >> 7U)> din{PIN_DIN1, PIN_DIN2, PIN_DIN4};
    std::array<pin_size_t, (pixels.size() >> 7U)> sclk{PIN_CLK1, PIN_CLK2, PIN_CLK4};

    std::array<std::array<uint8_t, 16U>, (pixels.size() >> 7U)> buffer{};

    void vkEnd(size_t chip);
    void vkSet(size_t chip, uint8_t byte);
    void vkStart(size_t chip);
};

extern DisplayService &Display;
