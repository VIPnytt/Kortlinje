#include "modes/GlitterMode.h"

#include "services/DisplayService.h"

void GlitterMode::handle()
{
    const size_t idx{static_cast<size_t>(random(static_cast<long>(Display::grid::pixels)))};
    DisplayService::setPixel(idx, !DisplayService::getPixel(idx));
}
