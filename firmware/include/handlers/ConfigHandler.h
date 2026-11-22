#pragma once

#include "modules/HandlerModule.h"

#include <LittleFS.h>
#include <array>
#include <cstring>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

class ConfigHandler : public HandlerModule
{
private:
    template <typename T> static std::false_type isView(std::type_identity<T>);
    template <typename T, size_t N> static std::true_type isView(std::type_identity<std::span<T, N>>);
    template <typename CharT, typename Traits>
    static std::true_type isView(std::type_identity<std::basic_string_view<CharT, Traits>>);

    template <typename T>
    static constexpr bool isStorable{std::is_trivially_copyable_v<T> && !std::is_pointer_v<T> &&
                                     !std::is_member_pointer_v<T> && !std::is_null_pointer_v<T> &&
                                     !std::is_array_v<T> && !std::ranges::view<std::remove_cv_t<T>> &&
                                     !std::is_convertible_v<const T &, std::string_view>};

    [[nodiscard]] static bool formatPath(std::span<char> out, std::string_view name, std::string_view key);

public:
    static bool get(std::string_view module, std::string_view key, std::string &value);
    static bool get(std::string_view module, std::string_view key, std::string_view &value) = delete;
    static bool remove(std::string_view module, std::string_view key);
    static bool set(std::string_view module, std::string_view key, std::string_view value);

    template <typename T>
        requires isStorable<T>
    static bool get(std::string_view module, std::string_view key, T &value)
    {
        std::array<char, 0b1U << 6U> path{};
        if (!formatPath(path, module, key))
        {
            return false;
        }
        File file{LittleFS.open(path.data(), "r")};
        if (!file)
        {
            return false;
        }
        if (file.size() != sizeof(T))
        {
            file.close();
            return false;
        }
        T _value{};
        const int length{file.read(reinterpret_cast<uint8_t *>(&_value), sizeof(T))};
        file.close();
        if (length >= 0 && static_cast<size_t>(length) == sizeof(T))
        {
            if (memcmp(&value, &_value, sizeof(T)) != 0)
            {
                value = _value;
            }
            return true;
        }
        return false;
    }

    template <typename T>
        requires isStorable<T>
    static bool set(std::string_view module, std::string_view key, const T &value)
    {
        std::array<char, 0b1U << 6U> path{};
        if (!formatPath(path, module, key))
        {
            return false;
        }
        File file{LittleFS.open(path.data(), "w")};
        if (!file)
        {
            return false;
        }
        const size_t written{file.write(reinterpret_cast<const uint8_t *>(&value), sizeof(T))};
        file.close();
        return written == sizeof(T);
    }
};
