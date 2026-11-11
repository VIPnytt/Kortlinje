#pragma once

#include "config/IkeaKortlinje.h"
#include "config/secrets.h"
#include "modules/ServiceModule.h"

#include <array>
#include <span>

class DisplayService final : public ServiceModule
{
public:
    static constexpr std::string_view name{"Display"};

    explicit DisplayService() : ServiceModule(name) {};

    enum class Brightness : uint8_t
    {
        DUTY1 = 0b1000'1000U,
        DUTY2 = 0b1000'1001U,
        DUTY4 = 0b1000'1010U,
        DUTY10 = 0b1000'1011U,
        DUTY11 = 0b1000'1100U,
        DUTY12 = 0b1000'1101U,
        DUTY13 = 0b1000'1110U,
        DUTY14 = 0b1000'1111U,
    };

    enum class Symbol : uint8_t
    {
        AM,
        PM,
        ALARM1,
        SNOOZE,
        ALARM2,
        STATUS,
    };

    void begin();
    void handle();
    void drawEllipseOutline(float x, float y, float radius, bool lit);
    void drawEllipseSolid(float x, float y, float radius, bool lit);
    void drawRectangleOutline(size_t minX, size_t columns, size_t minY, size_t maxY, bool lit);
    void fillColumn(uint8_t x, bool lit);
    void fillRow(uint8_t y, bool lit);
    void fillRows(size_t minY, size_t rows, bool lit);
    void flush();
    void invertFrame();
    void setFrame(std::span<const bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> _frame);
    void transmit();

    static void drawLineHorizontal(uint8_t xMin, size_t columns, uint8_t y, bool lit);
    static void drawLineVertical(uint8_t x, uint8_t yMin, uint8_t yMax, bool lit);
    static void drawRectangleSolid(uint8_t minX, uint8_t columns, uint8_t minY, uint8_t maxY, bool lit);
    static void fillFrame(bool lit);
    static void onHomeAssistant(JsonDocument &discovery, std::string topic, std::string unique);
    static void onReceive(JsonObjectConst payload, std::string_view source);
    static void setBrightness(Brightness _brightness);
    static void setPixel(uint8_t x, uint8_t y, bool lit);
    static void setPixel(size_t idx, bool lit);
    static void setPixel(Symbol symbol, bool lit);
    static void setPower(bool enable);

    [[nodiscard]] bool getPixel(size_t idx);
    [[nodiscard]] bool getPixel(uint8_t x, uint8_t y);
    [[nodiscard]] static bool getPixel(Symbol symbol);
    [[nodiscard]] static bool getPower();

    [[nodiscard]] static std::span<const bool, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> getFrame();

    [[nodiscard]] static Brightness getBrightness();

private:
    static constexpr std::array<uint16_t, ((GRID_COLUMNS * GRID_ROWS) + 0x7FU) & ~0x7FU> pixels{LED_MAP};

    static constexpr size_t chips{pixels.size() >> 7U};

    static inline bool pending{false};
    static inline bool power{false};
    static inline bool render{false};

    static inline std::array<bool, pixels.size()> frame{};

    std::array<pin_size_t, chips> sclk{PIN_CLK1, PIN_CLK2, PIN_CLK3};
    std::array<pin_size_t, chips> sda{PIN_DA1, PIN_DA2, PIN_DA3};

    alignas(4U) std::array<std::array<uint8_t, 16U>, chips> vkBuffer{};

    alignas(4U) static inline std::array<std::array<uint32_t, 19U>, chips> txBuffer{};

    static inline Brightness brightness{Brightness::DUTY14};
};
