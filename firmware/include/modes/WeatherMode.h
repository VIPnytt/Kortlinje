#pragma once

#include "modules/ModeModule.h"

#include <array>
#include <cstdint>
#include <optional>
#include <pico/time.h>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

class WeatherMode final : public ModeModule
{
private:
    static constexpr std::array<const char *, 1U> urlsOpenMeteo{
#if TEMPERATURE_CELSIUS || TEMPERATURE_KELVIN
        "https://api.open-meteo.com/v1/forecast?latitude=" LATITUDE "&longitude=" LONGITUDE
        "&current=temperature_2m,weather_code&temperature_unit=celsius",
#elif TEMPERATURE_FAHRENHEIT
        "https://api.open-meteo.com/v1/forecast?latitude=" LATITUDE "&longitude=" LONGITUDE
        "&current=temperature_2m,weather_code&temperature_unit=fahrenheit",
#else
        "https://api.open-meteo.com/v1/forecast?latitude=" LATITUDE "&longitude=" LONGITUDE
        "&current=temperature_2m,weather_code",
#endif // TEMPERATURE_CELSIUS || TEMPERATURE_KELVIN
    };
    static constexpr std::array<const char *, 2U> urlsWttrIn{
        "https://wttr.in/" LATITUDE "," LONGITUDE "?format=j1",
        "https://wttr.in/" LATITUDE "," LONGITUDE "?format=j2",
    };
    static constexpr std::array<const char *, 2U> urlsYr{
        "https://api.met.no/weatherapi/locationforecast/2.0/complete?lat=" LATITUDE "&lon=" LONGITUDE,
        "https://api.met.no/weatherapi/nowcast/2.0/complete?lat=" LATITUDE "&lon=" LONGITUDE,
    };
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

    struct Provider
    {
        std::string_view name;
        std::span<const char *const> urls;
        bool (*parse)(std::string_view body, std::optional<Condition> &condition, std::optional<int16_t> &temperature);
    };

    volatile bool pending{false};

    alarm_id_t alarmId{0};

    static const std::array<Provider, 3U> providers;

    std::optional<int16_t> temperature{};

    std::optional<Condition> condition{};

    std::vector<const char *> urls{};

    const Provider *provider{nullptr};

    void cancelAlarm();
    void draw();
    void schedule(uint32_t delayMs);
    void setProvider(std::string_view providerName);
    void transmit();

    [[nodiscard]] static bool parseOpenMeteo(std::string_view body, std::optional<Condition> &cond,
                                             std::optional<int16_t> &temp);
    [[nodiscard]] static bool parseWttrIn(std::string_view body, std::optional<Condition> &cond,
                                          std::optional<int16_t> &temp);
    [[nodiscard]] static bool parseYr(std::string_view body, std::optional<Condition> &cond,
                                      std::optional<int16_t> &temp);

    [[nodiscard]] static int fetch(const char *url, std::vector<char> &body);

    [[nodiscard]] static int64_t onAlarm(alarm_id_t id, void *userData);

    [[nodiscard]] static std::span<const uint16_t> getSign(Condition cond);

    template <typename T>
    [[nodiscard]] static std::optional<Condition>
    getCondition(T code, std::type_identity_t<std::span<const std::pair<Condition, std::span<const T>>>> codesets);

public:
    static constexpr std::string_view name{"Weather"};

    explicit WeatherMode() : ModeModule(name) {}
    ~WeatherMode() override;

    void begin() override;
    void handle() override;
    void end() override;
    void onReceive(JsonObjectConst payload, std::string_view source) override;
};
