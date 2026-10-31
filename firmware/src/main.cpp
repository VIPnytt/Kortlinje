
#include "services/DeviceService.h"

DeviceService device{};

void setup() { device.begin0(); }

void setup1() { device.begin1(); }

void loop() { device.handle0(); }

void loop1() { device.handle1(); }
