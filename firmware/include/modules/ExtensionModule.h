#pragma once

#include <string_view>

class ExtensionModule
{
protected:
    explicit ExtensionModule(std::string_view name) : name(name) {};

public:
    virtual ~ExtensionModule() = default;

    ExtensionModule(const ExtensionModule &) = delete;
    ExtensionModule &operator=(const ExtensionModule &) = delete;
    ExtensionModule(ExtensionModule &&) = delete;
    ExtensionModule &operator=(ExtensionModule &&) = delete;

    const std::string_view name{};

    virtual void begin();
    virtual void handle();
};
