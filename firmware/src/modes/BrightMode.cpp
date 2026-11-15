#include "modes/BrightMode.h"

#include "services/DisplayService.h"

void BrightMode::begin() { DisplayService::fillFrame(true); }
