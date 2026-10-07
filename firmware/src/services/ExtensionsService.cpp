#include "services/ExtensionsService.h"

#include "services/DeviceService.h"

void ExtensionsService::begin()
{
    for (ExtensionModule *extension : modules)
    {
        extension->begin();
    }
}

std::span<ExtensionModule *const> ExtensionsService::getAll() { return modules; }

void ExtensionsService::handle()
{
    for (ExtensionModule *extension : Extensions.getAll())
    {
        extension->handle();
    }
}

ExtensionsService &ExtensionsService::getInstance()
{
    static ExtensionsService instance;
    return instance;
}

ExtensionsService &Extensions{ExtensionsService::getInstance()};
