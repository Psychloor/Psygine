// SPDX-FileCopyrightText: 2025 Kevin Blomqvist
// SPDX-License-Identifier: MIT

#ifndef PSYGINE_SDL_RAII_HPP
#define PSYGINE_SDL_RAII_HPP

#include <memory>

#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_video.h>

namespace psygine::core
{
    using SdlWindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
    using SdlMetalViewPtr = std::unique_ptr<SDL_MetalView, decltype(&SDL_Metal_DestroyView)>;

    namespace sdl_raii
    {
        template <typename... Args>
        [[nodiscard]] SdlWindowPtr CreateWindow(Args&&... args)
        {
            return SdlWindowPtr(SDL_CreateWindow(std::forward<Args>(args)...), &SDL_DestroyWindow);
        }

        template <typename... Args>
        [[nodiscard]] SdlMetalViewPtr CreateMetalView(Args&&... args)
        {
            return SdlMetalViewPtr(SDL_Metal_CreateView(std::forward<Args>(args)...), &SDL_Metal_DestroyView);
        }
    }
}

#endif //PSYGINE_SDL_RAII_HPP
