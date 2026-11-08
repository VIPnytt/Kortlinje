#pragma once

#include "modules/HandlerModule.h"

#include <array>
#include <type_traits>

class EmbedHandler final : public HandlerModule
{
public:
    /** @brief Copy embedded bytes into a compile-time array without adding a terminator. */
    template <typename T, std::size_t N>
    [[nodiscard]] static consteval auto embed(const std::type_identity_t<T> (&&raw)[N])
    {
        return std::to_array(raw);
    }
};
