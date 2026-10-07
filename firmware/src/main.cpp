
#include "services/DeviceService.h"

void setup() { Device.begin0(); }

void setup1() { Device.begin1(); }

void loop() { Device.handle0(); }

void loop1() { Device.handle1(); }
