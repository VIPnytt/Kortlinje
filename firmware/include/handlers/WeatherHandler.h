#pragma once

#include <Arduino.h>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace BearSSL
{
class CertStore;
}

class WeatherHandler
{
public:
    virtual ~WeatherHandler() = default;

    WeatherHandler(const WeatherHandler &) = delete;
    WeatherHandler &operator=(const WeatherHandler &) = delete;
    WeatherHandler(WeatherHandler &&) = delete;
    WeatherHandler &operator=(WeatherHandler &&) = delete;

    enum class Condition : uint8_t
    {
        CLEAR,
        CLOUDY,
        CLOUDY_PARTLY,
        FOG,
        RAIN,
        SNOW,
        THUNDER,
        WIND,
    };

    const std::string_view name{};

    const unsigned long interval{0b1U << 19U};

    virtual void update(std::optional<Condition> &condition, std::optional<int16_t> &temperature,
                        unsigned long &lastMillis) = 0;

    [[nodiscard]] std::variant<std::span<const uint8_t>, std::span<const uint16_t>> getSign(Condition condition);

    static void setCertStore(BearSSL::CertStore *store) { certStore = store; }

protected:
    explicit WeatherHandler(std::string_view name) : name(name) {};

    static constexpr std::array<uint16_t, 9U> bitmapClear{
        0b000111000U,
        0b011111110U,
        0b011111110U,
        0b111111111U,
        0b111111111U,
        0b111111111U,
        0b011111110U,
        0b011111110U,
        0b000111000U,
    };
    static constexpr std::array<uint16_t, 6U> bitmapCloudy{
        0b000001110000000U,
        0b000011001111000U,
        0b001110000000110U,
        0b011000000000011U,
        0b110001000000011U,
        0b011111111111110U,
    };
    static constexpr std::array<uint16_t, 8U> bitmapCloudyPartly{
        0b000000000011100U,
        0b000000000111110U,
        0b000001110111110U,
        0b000011001111110U,
        0b001110000000111U,
        0b011000000000011U,
        0b110001000000011U,
        0b011111111111110U,
    };
    static constexpr std::array<uint16_t, 6U> bitmapFog{
        0b000000111111111U,
        0b011111111000000U,
        0b000011111111111U,
        0b111111111000000U,
        0b000001111111100U,
        0b011111111100000U,
    };
    static constexpr std::array<uint16_t, 9U> bitmapRain{
        0b000001110000000U,
        0b000011001111000U,
        0b001110000000110U,
        0b011000000000011U,
        0b110001000000011U,
        0b011111111111110U,
        0b001001001001000U,
        0b001001001001000U,
        0b001001001001000U,
    };
    static constexpr std::array<uint16_t, 9U> bitmapSnow{
        0b000001110000000U,
        0b000011001111000U,
        0b001110000000110U,
        0b011000000000011U,
        0b110001000000011U,
        0b011111111111110U,
        0b000100010001000U,
        0b010001000100010U,
        0b000100010001000U,
    };
    static constexpr std::array<uint16_t, 9U> bitmapThunder{
        0b000001110000000U,
        0b000011001111000U,
        0b001110000000110U,
        0b011000001000011U,
        0b110001010000011U,
        0b011110101111110U,
        0b000001111000000U,
        0b000000100000000U,
        0b000001000000000U,
    };
    static constexpr std::array<uint16_t, 9U> bitmapWind{
        0b1011110000000U,
        0b1010001111000U,
        0b1110000000111U,
        0b1010001111000U,
        0b1011110000000U,
        0b1000000000000U,
        0b1000000000000U,
        0b1000000000000U,
        0b1000000000000U,
    };

    const char *host{nullptr};
    const char *path{nullptr};
    const char *query{nullptr};

    static inline BearSSL::CertStore *certStore{nullptr};

    [[nodiscard]] int fetch(std::vector<char> &body, unsigned long &lastMillis);

    [[nodiscard]] std::optional<Condition>
    getCondition(std::string_view code,
                 std::span<const std::pair<Condition, std::span<const std::string_view>>> codesets);

    template <typename T>
        requires std::is_unsigned_v<T>
    [[nodiscard]] std::optional<Condition>
    getCondition(T code, std::span<const std::pair<Condition, std::span<const T>>> codesets);
};
