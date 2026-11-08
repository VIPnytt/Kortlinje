#include "modes/WeatherMode.h"

#include "config/version.h"
#include "fonts/KortlinjeFont.h"
#include "fonts/SmallFont.h"
#include "handlers/BitmapHandler.h"
#include "handlers/CertificateHandler.h"
#include "handlers/ConfigHandler.h"
#include "handlers/TextHandler.h"
#include "middlewares/WeatherMiddleware.h"
#include "services/DeviceService.h"
#include "services/DisplayService.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <string>

const std::array<WeatherMode::Provider, 3U> WeatherMode::providers{{
    {"Open-Meteo", urlsOpenMeteo, &WeatherMode::parseOpenMeteo},
    {"Wttr.in", urlsWttrIn, &WeatherMode::parseWttrIn},
    {"Yr", urlsYr, &WeatherMode::parseYr},
}};

WeatherMode::~WeatherMode() { cancelAlarm(); }

void WeatherMode::begin()
{
    std::string _provider{};
    if (ConfigHandler::get(name, "provider", _provider))
    {
        setProvider(_provider);
    }
    if (provider == nullptr)
    {
        setProvider(providers[0U].name);
    }
}

void WeatherMode::handle()
{
    if (!pending || !WiFi.isConnected())
    {
        return;
    }
    pending = false;
    if (urls.empty())
    {
        return;
    }
    std::vector<char> body{};
    const int status{fetch(urls.back(), body)};
    if (status == 200)
    {
        if (provider->parse({body.data(), body.size()}, condition, temperature) && condition.has_value() &&
            temperature.has_value())
        {
            draw();
            transmit();
            schedule(0b1UL << 19U);
            return;
        }
        urls.pop_back();
    }
    else if (status >= 400 && status < 500 && status != 408 && status != 429)
    {
        urls.pop_back();
    }
    else
    {
        schedule(0b1UL << 16U);
        return;
    }
    if (!urls.empty())
    {
        schedule(0b1UL << 10U);
    }
}

void WeatherMode::end() { cancelAlarm(); }

void WeatherMode::cancelAlarm()
{
    if (alarmId > 0)
    {
        cancel_alarm(alarmId);
        alarmId = 0;
    }
    pending = false;
}

void WeatherMode::draw()
{
    const BitmapHandler bitmap(getSign(condition.value()));
    const KortlinjeFont font;
#if TEMPERATURE_KELVIN
    const TextHandler text(std::to_string(temperature.value()), font);
#else
    const TextHandler text(std::format("{:d}°", temperature.value()), font);
#endif // TEMPERATURE_KELVIN
    const uint8_t textWidth{text.getWidth()};
    const uint8_t marginsX{static_cast<uint8_t>(max(0U, GRID_COLUMNS - bitmap.getWidth() - textWidth) / 3U)};
    DisplayService::fillFrame(false);
    bitmap.draw(marginsX, (GRID_ROWS - bitmap.getHeight()) / 2U);
    text.draw(GRID_COLUMNS - textWidth - marginsX - 1U, (GRID_ROWS - text.getHeight()) / 2U);
}

int WeatherMode::fetch(const char *url, std::vector<char> &body)
{
    BearSSL::WiFiClientSecure secure{};
    secure.setTrustAnchors(&CertificateHandler::list());
    HTTPClient http{};
    http.setUserAgent("Kortlinje/" VERSION " (RP2350; +https://github.com/VIPnytt/Kortlinje)");
    if (!http.begin(secure, url))
    {
        return -1;
    }
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");
    const int status{http.GET()};
    if (status == 200)
    {
        const int length{http.getSize()};
        if (length > 0)
        {
            body.reserve(static_cast<size_t>(length));
        }
        WeatherMiddleware writer(body);
        http.writeToStream(&writer);
    }
    http.end();
    return status;
}

template <typename T>
std::optional<WeatherMode::Condition>
WeatherMode::getCondition(T code,
                          std::type_identity_t<std::span<const std::pair<Condition, std::span<const T>>>> codesets)
{
    for (const auto &[cond, codes] : codesets)
    {
        if (std::find(codes.begin(), codes.end(), code) != codes.end())
        {
            return cond;
        }
    }
    return std::nullopt;
}

std::span<const uint16_t> WeatherMode::getSign(Condition cond)
{
    switch (cond)
    {
    case Condition::CLEAR:
        return bitmapClear;
    case Condition::CLOUDY:
        return bitmapCloudy;
    case Condition::CLOUDY_PARTLY:
        return bitmapCloudyPartly;
    case Condition::FOG:
        return bitmapFog;
    case Condition::RAIN:
        return bitmapRain;
    case Condition::SNOW:
        return bitmapSnow;
    case Condition::THUNDER:
        return bitmapThunder;
    case Condition::WIND:
        return bitmapWind;
    }
    return {};
}

int64_t __time_critical_func(WeatherMode::onAlarm)(alarm_id_t id, void *user_data)
{
    static_cast<void>(id);
    auto *self{static_cast<WeatherMode *>(user_data)};
    self->alarmId = 0L;
    self->pending = true;
    return 0LL;
}

void WeatherMode::onReceive(JsonObjectConst payload, std::string_view source)
{
    static_cast<void>(source);
    if (payload["provider"].is<std::string_view>())
    {
        setProvider(payload["provider"].as<std::string_view>());
    }
}

bool WeatherMode::parseOpenMeteo(std::string_view body, std::optional<Condition> &cond, std::optional<int16_t> &temp)
{
    static constexpr std::array<uint8_t, 1U> codesClear{0U};
    static constexpr std::array<uint8_t, 1U> codesCloudy{3U};
    static constexpr std::array<uint8_t, 2U> codesCloudyPartly{1U, 2U};
    static constexpr std::array<uint8_t, 2U> codesFog{45U, 48U};
    static constexpr std::array<uint8_t, 13U> codesRain{
        51U, 53U, 55U, 56U, 57U, 61U, 63U, 65U, 66U, 67U, 80U, 81U, 82U};
    static constexpr std::array<uint8_t, 6U> codesSnow{71U, 73U, 75U, 77U, 85U, 86U};
    static constexpr std::array<uint8_t, 3U> codesThunder{95U, 96U, 99U};
    static constexpr std::array<std::pair<Condition, std::span<const uint8_t>>, 7U> codesets{{
        {Condition::CLEAR, codesClear},
        {Condition::CLOUDY, codesCloudy},
        {Condition::CLOUDY_PARTLY, codesCloudyPartly},
        {Condition::FOG, codesFog},
        {Condition::RAIN, codesRain},
        {Condition::SNOW, codesSnow},
        {Condition::THUNDER, codesThunder},
    }};
    JsonDocument filter{};
    filter["current"]["temperature_2m"].set(true);
    filter["current"]["weather_code"].set(true);
    JsonDocument doc{};
    if (deserializeJson(doc, body, DeserializationOption::Filter(filter)) == DeserializationError::Code::Ok &&
        doc["current"]["temperature_2m"].is<float>() && doc["current"]["weather_code"].is<uint8_t>())
    {
        cond = getCondition<uint8_t>(doc["current"]["weather_code"].as<uint8_t>(), codesets);
#if TEMPERATURE_KELVIN
        temp = static_cast<int16_t>(lroundf(273.15F + doc["current"]["temperature_2m"].as<float>()));
#else
        temp = static_cast<int16_t>(lroundf(doc["current"]["temperature_2m"].as<float>()));
#endif // TEMPERATURE_KELVIN
        return true;
    }
    return false;
}

bool WeatherMode::parseWttrIn(std::string_view body, std::optional<Condition> &cond, std::optional<int16_t> &temp)
{
    static constexpr std::array<uint16_t, 1U> codesClear{113U};
    static constexpr std::array<uint16_t, 2U> codesCloudy{119U, 122U};
    static constexpr std::array<uint16_t, 1U> codesCloudyPartly{116U};
    static constexpr std::array<uint16_t, 3U> codesFog{143U, 248U, 260U};
    static constexpr std::array<uint16_t, 19U> codesRain{
        176U,
        293U,
        296U,
        299U,
        302U,
        305U,
        308U,
        311U,
        314U,
        353U,
        356U,
        359U,
        386U,
        389U,
        263U,
        266U,
        281U,
        284U,
        185U,
    };
    static constexpr std::array<uint16_t, 14U> codesSnow{
        179U, 227U, 323U, 326U, 329U, 332U, 335U, 338U, 368U, 371U, 392U, 395U, 230U, 350U};
    static constexpr std::array<uint16_t, 5U> codesThunder{200U, 386U, 389U, 392U, 395U};
    static constexpr std::array<std::pair<Condition, std::span<const uint16_t>>, 7U> codesets{{
        {Condition::CLEAR, codesClear},
        {Condition::CLOUDY, codesCloudy},
        {Condition::CLOUDY_PARTLY, codesCloudyPartly},
        {Condition::FOG, codesFog},
        {Condition::RAIN, codesRain},
        {Condition::SNOW, codesSnow},
        {Condition::THUNDER, codesThunder},
    }};
    JsonDocument filter{};
#if TEMPERATURE_FAHRENHEIT
    filter["current_condition"][0U]["temp_F"].set(true);
#else
    filter["current_condition"][0U]["temp_C"].set(true);
#endif // TEMPERATURE_FAHRENHEIT
    filter["current_condition"][0U]["weatherCode"].set(true);
    JsonDocument doc{};
    if (deserializeJson(doc, body, DeserializationOption::Filter(filter)) == DeserializationError::Code::Ok &&
#if TEMPERATURE_FAHRENHEIT
        doc["current_condition"][0U]["temp_F"].is<std::string_view>() &&
#else
        doc["current_condition"][0U]["temp_C"].is<std::string_view>() &&
#endif // TEMPERATURE_FAHRENHEIT
        doc["current_condition"][0U]["weatherCode"].is<std::string_view>())
    {
        cond = getCondition<uint16_t>(doc["current_condition"][0U]["weatherCode"].as<uint16_t>(), codesets);
#if TEMPERATURE_FAHRENHEIT
        temp = doc["current_condition"][0U]["temp_F"].as<int16_t>();
#elif TEMPERATURE_KELVIN
        temp = static_cast<int16_t>(273 + doc["current_condition"][0U]["temp_C"].as<int>());
#else
        temp = doc["current_condition"][0U]["temp_C"].as<int16_t>();
#endif // TEMPERATURE_FAHRENHEIT
        return true;
    }
    return false;
}

bool WeatherMode::parseYr(std::string_view body, std::optional<Condition> &cond, std::optional<int16_t> &temp)
{
    static constexpr std::array<std::string_view, 6U> codesClear{
        "clearsky_day",
        "clearsky_night",
        "clearsky_polartwilight",
        "fair_day",
        "fair_night",
        "fair_polartwilight",
    };
    static constexpr std::array<std::string_view, 1U> codesCloudy{"cloudy"};
    static constexpr std::array<std::string_view, 3U> codesCloudyPartly{
        "partlycloudy_day",
        "partlycloudy_night",
        "partlycloudy_polartwilight",
    };
    static constexpr std::array<std::string_view, 1U> codesFog{"fog"};
    static constexpr std::array<std::string_view, 12U> codesRain{
        "heavyrain",
        "heavyrainshowers_day",
        "heavyrainshowers_night",
        "heavyrainshowers_polartwilight",
        "lightrain",
        "lightrainshowers_day",
        "lightrainshowers_night",
        "lightrainshowers_polartwilight",
        "rain",
        "rainshowers_day",
        "rainshowers_night",
        "rainshowers_polartwilight",
    };
    static constexpr std::array<std::string_view, 24U> codesSnow{
        "heavysleet",
        "heavysleetshowers_day",
        "heavysleetshowers_night",
        "heavysleetshowers_polartwilight",
        "heavysnow",
        "heavysnowshowers_day",
        "heavysnowshowers_night",
        "heavysnowshowers_polartwilight",
        "lightsleet",
        "lightsleetshowers_day",
        "lightsleetshowers_night",
        "lightsleetshowers_polartwilight",
        "lightsnow",
        "lightsnowshowers_day",
        "lightsnowshowers_night",
        "lightsnowshowers_polartwilight",
        "sleet",
        "sleetshowers_day",
        "sleetshowers_night",
        "sleetshowers_polartwilight",
        "snow",
        "snowshowers_day",
        "snowshowers_night",
        "snowshowers_polartwilight",
    };
    static constexpr std::array<std::string_view, 42U> codesThunder{
        "heavyrainandthunder",
        "heavyrainshowersandthunder_day",
        "heavyrainshowersandthunder_night",
        "heavyrainshowersandthunder_polartwilight",
        "heavysleetandthunder",
        "heavysleetshowersandthunder_day",
        "heavysleetshowersandthunder_night",
        "heavysleetshowersandthunder_polartwilight",
        "heavysnowandthunder",
        "heavysnowshowersandthunder_day",
        "heavysnowshowersandthunder_night",
        "heavysnowshowersandthunder_polartwilight",
        "lightrainandthunder",
        "lightrainshowersandthunder_day",
        "lightrainshowersandthunder_night",
        "lightrainshowersandthunder_polartwilight",
        "lightsleetandthunder",
        "lightsnowandthunder",
        "lightsleetshowersandthunder_day",
        "lightsleetshowersandthunder_night",
        "lightsleetshowersandthunder_polartwilight",
        "lightssleetshowersandthunder_day",
        "lightssleetshowersandthunder_night",
        "lightssleetshowersandthunder_polartwilight",
        "lightsnowshowersandthunder_day",
        "lightsnowshowersandthunder_night",
        "lightsnowshowersandthunder_polartwilight",
        "lightssnowshowersandthunder_day",
        "lightssnowshowersandthunder_night",
        "lightssnowshowersandthunder_polartwilight",
        "rainandthunder",
        "rainshowersandthunder_day",
        "rainshowersandthunder_night",
        "rainshowersandthunder_polartwilight",
        "sleetandthunder",
        "sleetshowersandthunder_day",
        "sleetshowersandthunder_night",
        "sleetshowersandthunder_polartwilight",
        "snowandthunder",
        "snowshowersandthunder_day",
        "snowshowersandthunder_night",
        "snowshowersandthunder_polartwilight",
    };
    static constexpr std::array<std::pair<Condition, std::span<const std::string_view>>, 7U> codesets{{
        {Condition::CLEAR, codesClear},
        {Condition::CLOUDY, codesCloudy},
        {Condition::CLOUDY_PARTLY, codesCloudyPartly},
        {Condition::FOG, codesFog},
        {Condition::RAIN, codesRain},
        {Condition::SNOW, codesSnow},
        {Condition::THUNDER, codesThunder},
    }};
    JsonDocument filter{};
    filter["properties"]["timeseries"][0U]["data"]["instant"]["details"]["air_temperature"].set(true);
    filter["properties"]["timeseries"][0U]["data"]["next_1_hours"]["summary"]["symbol_code"].set(true);
    JsonDocument doc{};
    if (deserializeJson(doc, body, DeserializationOption::Filter(filter)) == DeserializationError::Code::Ok &&
        doc["properties"]["timeseries"][0U]["data"]["instant"]["details"]["air_temperature"].is<float>() &&
        doc["properties"]["timeseries"][0U]["data"]["next_1_hours"]["summary"]["symbol_code"].is<std::string_view>())
    {
        cond = getCondition(doc["properties"]["timeseries"][0U]["data"]["next_1_hours"]["summary"]["symbol_code"]
                                .as<std::string_view>(),
                            codesets);
        const float celsius{
            doc["properties"]["timeseries"][0U]["data"]["instant"]["details"]["air_temperature"].as<float>()};
#if TEMPERATURE_FAHRENHEIT
        temp = static_cast<int16_t>(lroundf(celsius * 9.0F / 5.0F) + 32L);
#elif TEMPERATURE_KELVIN
        temp = static_cast<int16_t>(lroundf(273.15F + celsius));
#else
        temp = static_cast<int16_t>(lroundf(celsius));
#endif // TEMPERATURE_FAHRENHEIT
        return true;
    }
    return false;
}

void WeatherMode::schedule(uint32_t delayMs)
{
    cancelAlarm();
    alarmId = add_alarm_in_ms(delayMs, &onAlarm, this, true);
}

void WeatherMode::setProvider(std::string_view providerName)
{
    if (provider != nullptr && provider->name == providerName)
    {
        return;
    }
    for (const Provider &entry : providers)
    {
        if (entry.name == providerName)
        {
            cancelAlarm();
            provider = &entry;
            urls.assign(entry.urls.begin(), entry.urls.end());
            condition.reset();
            temperature.reset();
            DisplayService::fillFrame(false);
            SmallFont font{};
            TextHandler text(std::string(provider->name), font);
            text.draw((GRID_COLUMNS - min(GRID_COLUMNS, text.getWidth())) / 2U, (GRID_ROWS - text.getHeight()) / 2U);
            pending = true;
            ConfigHandler::set(name, "provider", provider->name);
            transmit();
            return;
        }
    }
}

void WeatherMode::transmit()
{
    JsonDocument doc{};
    doc["provider"].set(provider->name);
    JsonArray _providers{doc["providers"].to<JsonArray>()};
    for (const Provider &entry : providers)
    {
        _providers.add(entry.name);
    }
    DeviceService::transmit(doc.as<JsonObjectConst>(), name);
}
