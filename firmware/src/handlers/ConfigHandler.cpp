#include "handlers/ConfigHandler.h"

bool ConfigHandler::formatPath(std::span<char> out, std::string_view module, std::string_view key)
{
    const int written{snprintf(out.data(),
                               out.size(),
                               "/cfg_%.*s_%.*s.bin",
                               static_cast<int>(module.size()),
                               module.data(),
                               static_cast<int>(key.size()),
                               key.data())};
    return written > 0 && static_cast<size_t>(written) < out.size();
}

bool ConfigHandler::get(std::string_view module, std::string_view key, std::string &value)
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
    const size_t length{file.size()};
    std::string _value{};
    _value.resize(length);
    const int _length{file.read(reinterpret_cast<uint8_t *>(_value.data()), length)};
    file.close();
    if (_length >= 0 && static_cast<size_t>(_length) == length)
    {
        if (value != _value)
        {
            value = std::move(_value);
        }
        return true;
    }
    return false;
}

bool ConfigHandler::remove(std::string_view module, std::string_view key)
{
    std::array<char, 0b1U << 6U> path{};
    if (!formatPath(path, module, key))
    {
        return false;
    }
    return LittleFS.remove(path.data());
}

bool ConfigHandler::set(std::string_view module, std::string_view key, std::string_view value)
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
    const size_t written{file.write(reinterpret_cast<const uint8_t *>(value.data()), value.size())};
    file.close();
    return written == value.size();
}
