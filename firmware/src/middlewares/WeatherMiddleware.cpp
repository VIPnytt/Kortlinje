#include "middlewares/WeatherMiddleware.h"

WeatherMiddleware::WeatherMiddleware(std::vector<char> &dest) : target(dest) {}

int WeatherMiddleware::available() { return 0; }

void WeatherMiddleware::flush() {}

int WeatherMiddleware::peek() { return -1; }

int WeatherMiddleware::read() { return -1; }

size_t WeatherMiddleware::write(uint8_t byte)
{
    target.push_back(static_cast<char>(byte));
    return 1U;
}

size_t WeatherMiddleware::write(const uint8_t *buffer, size_t size)
{
    target.insert(target.end(), reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(buffer) + size);
    return size;
}
