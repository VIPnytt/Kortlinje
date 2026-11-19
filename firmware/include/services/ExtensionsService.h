#pragma once

#include "modules/ExtensionModule.h"
#include "modules/ServiceModule.h"

#include <array>
#include <span>
#include <string_view>

class ExtensionsService final : public ServiceModule
{
private:
    static constexpr std::string_view name{"Extensions"};

public:
    explicit ExtensionsService() : ServiceModule(name) {};

    void configure();
    void begin();
    void handle();

    [[nodiscard]] static std::span<ExtensionModule *const> getAll();

    [[nodiscard]] static ExtensionModule *const getExtension(std::string_view extensionName);
};
