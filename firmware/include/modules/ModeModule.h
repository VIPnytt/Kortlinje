#pragma once

#include <string_view>

class ModeModule
{
protected:
    explicit ModeModule(std::string_view name) : name(name) {};

public:
    virtual ~ModeModule() = default;

    ModeModule(const ModeModule &) = delete;
    ModeModule &operator=(const ModeModule &) = delete;
    ModeModule(ModeModule &&) = delete;
    ModeModule &operator=(ModeModule &&) = delete;

    const std::string_view name{};

    virtual void begin();
    virtual void handle();
    virtual void end();
};
