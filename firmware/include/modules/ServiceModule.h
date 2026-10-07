#pragma once

#include <string_view>

class ServiceModule
{
protected:
    explicit ServiceModule(std::string_view name) : name(name) {};

public:
    virtual ~ServiceModule() = default;

    ServiceModule(const ServiceModule &) = delete;
    ServiceModule &operator=(const ServiceModule &) = delete;
    ServiceModule(ServiceModule &&) = delete;
    ServiceModule &operator=(ServiceModule &&) = delete;

    const std::string_view name{};
};
