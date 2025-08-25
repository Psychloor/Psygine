//
// Created by blomq on 2025-08-25.
//

#include "runtime.hpp"

#include <iostream>

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bgfx/c99/bgfx.h"

namespace psygine::core
{
    Runtime::Runtime(RuntimeConfig config) :
        config_{std::move(config)}
    {}

    Runtime::~Runtime()
    {
        bgfx::shutdown();

        window_.reset();

        if (initializedGamepad)
        {
            SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        }

        SDL_Quit();
    }

    bool Runtime::initialize()
    {
        if (initialized_)
        {
            return true;
        }

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n' << std::flush;
            SDL_Quit();
            return false;
        }

        SDL_WindowFlags sdlWindowFlags = SDL_UINT64_C(0);
        if (config_.highDpi)
        {
            sdlWindowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
        }
        if (config_.fullscreen)
        {
            sdlWindowFlags |= SDL_WINDOW_FULLSCREEN;
        }
        if (config_.resizable)
        {
            sdlWindowFlags |= SDL_WINDOW_RESIZABLE;
        }
        if (config_.borderless)
        {
            sdlWindowFlags |= SDL_WINDOW_BORDERLESS;
        }

        // Scoped so you can't use raw window ptr
        {
            SDL_Window* window = SDL_CreateWindow(config_.title.c_str(), config_.width, config_.height, sdlWindowFlags);
            if (window == nullptr)
            {
                std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n' << std::flush;
                SDL_Quit();
                return false;
            }
            window_ = SdlWindowPtr(window, &SDL_DestroyWindow);
        }

        bgfx::PlatformData pd{};
        pd.ndt = nullptr;
        pd.nwh = nullptr;


        // Windows
#ifdef SDL_PLATFORM_WINDOWS
        pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                        SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif

        // X11
#ifdef SDL_PLATFORM_LINUX
        if (const char* drv = SDL_GetCurrentVideoDriver())
        {
            if (SDL_strcmp(drv, "x11") == 0)
            {
                pd.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                                SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
                pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(winwindow_.get()dow),
                                                SDL_PROP_WINDOW_X11_WINDOW_POINTER, nullptr);
            }
            else if (SDL_strcmp(drv, "wayland") == 0)
            {
                pd.ndt = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                                SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
                pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                                SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
            }
        }
#endif

        // Android
#ifdef SDL_PLATFORM_ANDROID
        pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                        SDL_PROP_WINDOW_ANDROID_NATIVE_WINDOW_POINTER, nullptr);
#endif

        // macOS (Metal)
#ifdef SDL_PLATFORM_MACOS
        pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                        SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif

        // iOS/tvOS (Metal)
#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_TVOS)
        // Assign CAMetalLayer* to nwh.
        pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                        SDL_PROP_WINDOW_ANDROID_NATIVE_WINDOW_POINTER, nullptr);
#endif

#ifdef SDL_PLATFORM_EMSCRIPTEN
        pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
                                        SDL_PROP_WINDOW_WEB_GLCONTEXT_POINTER, nullptr);
#endif


        // ReSharper disable once CppRedundantQualifierADL
        bgfx::setPlatformData(pd);

        bgfx::Init init{};
        init.vendorId = config_.graphicsApi == GraphicsApi::Software
            ? BGFX_PCI_ID_SOFTWARE_RASTERIZER
            : BGFX_PCI_ID_NONE;
        init.type = static_cast<bgfx::RendererType::Enum>(config_.graphicsApi);

        init.resolution.width = config_.width;
        init.resolution.height = config_.height;

        auto resetFlags = config_.bgfxCustomResetFlags;
        if (config_.vsync)
        {
            resetFlags |= BGFX_RESET_VSYNC;
        }
        if (config_.hdr10)
        {
            resetFlags |= BGFX_RESET_HDR10;
        }
        resetFlags |= static_cast<std::uint32_t>(config_.msaa);
        init.resolution.reset = resetFlags;


        initialized_ = bgfx::init(init);
        if (!initialized_)
        {
            std::cerr << "bgfx::init failed: " << getRendererName(bgfx::getRendererType()) << '\n' << std::flush;
            window_.reset();
            SDL_Quit();
            return false;
        }

        return initialized_;
    }

    bool Runtime::initializeGamepad()
    {
        if (initializedGamepad)
        {
            return true;
        }

        initializedGamepad = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
        return initializedGamepad;
    }

    void Runtime::run()
    {
        if (!initialized_)
        {
            std::cerr << "Runtime not initialized!" << '\n' << std::flush;
            return;
        }
    }

    Runtime::Runtime(Runtime&& other) noexcept :
        window_{std::move(other.window_)},
        config_{std::move(other.config_)}
    {}

    Runtime& Runtime::operator=(Runtime&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        window_ = std::move(other.window_);
        config_ = std::move(other.config_);
        return *this;
    }
}
