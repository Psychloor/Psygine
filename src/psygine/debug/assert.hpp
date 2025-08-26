//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

#ifndef PSYGINE_ASSERT_HPP
#define PSYGINE_ASSERT_HPP

#include <source_location>
#include <string_view>

#include <SDL3/SDL_assert.h>

namespace psygine::debug
{

    inline void Assert(const bool condition,
                       [[maybe_unused]] const std::string_view message,
                       [[maybe_unused]] std::source_location location =
                           std::source_location::current())
    {
        SDL_assert_always(condition); // NOLINT(*) - SDL knows what they're doing for all platforms
    }

#ifndef NDEBUG
    inline void DebugAssert(const bool condition,
                            [[maybe_unused]] const std::string_view message,
                            [[maybe_unused]] std::source_location location = std::source_location::current())
    {
        SDL_assert(condition); // NOLINT(*) - SDL knows what they're doing for all platforms
    }
#else
    constexpr void DebugAssert(const bool, const std::string_view,
                               std::source_location = std::source_location::current()) {}
#endif

} // namespace psygine::debug

#define PSYGINE_ASSERT(condition, message) ::psygine::debug::Assert((condition), (message))
#define PSYGINE_DEBUG_ASSERT(condition, message) ::psygine::debug::DebugAssert((condition), (message))


#endif //PSYGINE_ASSERT_HPP