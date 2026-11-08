#pragma once

#include <Stream.h>
#include <vector>

class WeatherMiddleware final : public Stream
{
private:
    std::vector<char> &target;

public:
    explicit WeatherMiddleware(std::vector<char> &dest);

    void flush() override;

    [[nodiscard]] int available() override;
    [[nodiscard]] int peek() override;
    [[nodiscard]] int read() override;

    [[nodiscard]] size_t write(uint8_t byte) override;
    [[nodiscard]] size_t write(const uint8_t *buffer, size_t size) override;
};
