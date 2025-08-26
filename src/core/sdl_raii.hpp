//
// Created by blomq on 2025-08-25.
//

#ifndef PSYGINE_SDL_RAII_HPP
#define PSYGINE_SDL_RAII_HPP

#include <memory>
#include <SDL3/SDL_metal.h>

#include <SDL3/SDL_video.h>

namespace psygine::core
{
    using SdlWindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
    using SdlMetalViewPtr = std::unique_ptr<SDL_MetalView, decltype(&SDL_Metal_DestroyView)>;
}

#endif //PSYGINE_SDL_RAII_HPP