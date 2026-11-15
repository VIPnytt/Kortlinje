#include "modes/GlitterMode.h"

#include "config/IkeaKortlinje.h"
#include "services/DisplayService.h"

void GlitterMode::handle()
{
    const size_t idx{static_cast<size_t>(random(static_cast<long>(GRID_COLUMNS * GRID_ROWS)))};
    DisplayService::setPixel(idx, !DisplayService::getPixel(idx));
}
