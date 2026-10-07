#pragma once

#include "modules/ServiceModule.h"

#include <Arduino.h>

class DeviceService final : public ServiceModule
{
private:
    explicit DeviceService() : ServiceModule("Device") {};

    float temperature{.0F};

    uint16_t voltage{0U};

    unsigned long lastMillis{0UL};

public:
    void begin0();
    void begin1();
    void handle0();
    void handle1();

    static DeviceService &getInstance();
};

extern DeviceService &Device;
