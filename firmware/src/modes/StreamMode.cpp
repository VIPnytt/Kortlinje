#include "modes/StreamMode.h"

#include "services/DisplayService.h"

#include <span>

StreamMode::~StreamMode() { udp.close(); }

/**
 * @brief Starts UDP listening on the Distributed Display Protocol port (4048).
 */
void StreamMode::begin()
{
    if (udp.listen(4'048U))
    {
        udp.onPacket(&onDistributedDisplayProtocol);
    }
}

/**
 * @brief Processes a 1-bit Distributed Display Protocol packet and updates the display frame.
 *
 * @param packet UDP packet containing a supported protocol header followed by a complete display frame.
 */
void StreamMode::onDistributedDisplayProtocol(AsyncUDPPacket packet)
{
    const std::span<const uint8_t> data{packet.data(), packet.length()};
    if (packet.length() == 10U + Display::grid::bytes && (data.front() & (0b1U << 4U)) == 0U)
    {
        DisplayService::setFrame(static_cast<std::span<const uint8_t, Display::grid::bytes>>(data.subspan(10U)));
    }
    else if (packet.length() == 14U + Display::grid::bytes && (data.front() & (0b1U << 4U)) != 0U)
    {
        DisplayService::setFrame(static_cast<std::span<const uint8_t, Display::grid::bytes>>(data.subspan(14U)));
    }
}
