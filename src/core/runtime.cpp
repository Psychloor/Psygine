//
// Created by blomq on 2025-08-25.
//

#include "runtime.hpp"

#include <algorithm>
#include <iostream>

#include "time.hpp"

#include  "SDL3/SDL_metal.h"

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"

namespace psygine::core
{
    Runtime::Runtime(RuntimeConfig config) :
        config_{std::move(config)}
    {}

    Runtime::~Runtime()
    {
        // Reverse order of initialization
        bgfx::shutdown();
        window_.reset();
        if (initializedGamepad_)
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

#if defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_TVOS)
        sdlWindowFlags |= SDL_WINDOW_METAL;
#endif

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
        // ReSharper disable once CppDFAConstantConditions
        if (!populatePlatformData(pd))
        {
            // ReSharper disable once CppDFAUnreachableCode
            std::cerr << "populatePlatformData failed" << '\n' << std::flush;
            window_.reset();
            SDL_Quit();
            return false;
        }
        // ReSharper disable once CppRedundantQualifierADL
        bgfx::setPlatformData(pd);

        bgfx::Init init{};
        init.vendorId = config_.graphicsApi == GraphicsApi::Software
            ? BGFX_PCI_ID_SOFTWARE_RASTERIZER
            : BGFX_PCI_ID_NONE;
        init.type = static_cast<bgfx::RendererType::Enum>(config_.graphicsApi);

        init.resolution.width = config_.width;
        init.resolution.height = config_.height;
        init.deviceId = config_.gpuDeviceId;
        init.resolution.reset = bgfxResetFlags();

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
        if (initializedGamepad_)
        {
            return true;
        }

        initializedGamepad_ = SDL_InitSubSystem(SDL_INIT_GAMEPAD);
        return initializedGamepad_;
    }

    void Runtime::run()
    {
        if (!initialized_)
        {
            std::cerr << "Runtime not initialized!" << '\n' << std::flush;
            return;
        }
        if (running_)
        {
            std::cerr << "Runtime already running!" << '\n' << std::flush;
            return;
        }

        if (config_.debug)
        {
            bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
        }

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, config_.rgbaClearColor, 1.0F, 0);
        bgfx::setViewRect(0, 0, 0, config_.width, config_.height);

        running_ = true;

        utils::time::types::TimePoint now = utils::time::Now();

        double accumulator = 0.0;
        std::size_t updatesThisFrame = 0;
        double const fixedTimestep = config_.fixedTimestep.count();
        double const maxTimestep = config_.maxTimestep.count();

        while (running_)
        {
            handleEvents();

            double deltaTime = utils::time::ElapsedSinceSeconds(now);
            now = utils::time::Now();

            // Protect some against lag spikes and all
            deltaTime = std::min(deltaTime, maxTimestep);
            accumulator += deltaTime;

            updatesThisFrame = 0;
            while (accumulator >= fixedTimestep && updatesThisFrame < config_.maxUpdatesPerTick)
            {
                accumulator -= fixedTimestep;
                ++updatesThisFrame;
                fixedUpdate(fixedTimestep);
            }

            if (accumulator >= fixedTimestep)
            {
                accumulator = std::fmod(accumulator, fixedTimestep);
            }

            update(deltaTime);

            const auto interpolationFactor = accumulator / fixedTimestep;
            render(interpolationFactor);
        }
    }

    void Runtime::setIsRunning(const bool running)
    {
        running_ = running;
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

    bool Runtime::onQuitRequested()
    {
        return true;
    }

    void Runtime::handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT: if (onQuitRequested())
                    {
                        running_ = false;
                    }
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                {
                    const auto width = event.window.data1;
                    const auto height = event.window.data2;
                    bgfx::reset(width, height, bgfxResetFlags());
                    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
                    onEvent(event);
                }
                break;

                default: onEvent(event);
                    break;
            }
        }
    }

    void Runtime::fixedUpdate(const double deltaTime)
    {
        onFixedUpdate(deltaTime);
    }

    void Runtime::update(const double deltaTime)
    {
        onUpdate(deltaTime);
    }

    void Runtime::render(const double interpolation)
    {
        bgfx::touch(0);

        onRender(interpolation);

        bgfx::frame();
    }

    std::uint32_t Runtime::bgfxResetFlags() const
    {
        std::uint32_t resetFlags = config_.bgfxCustomResetFlags;
        if (config_.vsync)
        {
            resetFlags |= BGFX_RESET_VSYNC;
        }
        if (config_.hdr10)
        {
            resetFlags |= BGFX_RESET_HDR10;
        }
        resetFlags |= static_cast<std::uint32_t>(config_.msaa);

        return resetFlags;
    }

    bool Runtime::populatePlatformData(bgfx::PlatformData& pd)
    {
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
                pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()),
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
        metalView_ = SdlMetalViewPtr(SDL_Metal_CreateView(window_.get()), &SDL_Metal_DestroyView);
        if (!metalView_)
        {
            std::cerr << "SDL_Metal_CreateView failed" << SDL_GetError() << '\n' << std::flush;
            return false;
        }
        pd.nwh = SDL_Metal_GetLayer(metalView_.get());

#endif

        // iOS/tvOS (Metal)
#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_TVOS)
        metalView_ = SdlMetalViewPtr(SDL_Metal_CreateView(window_.get()), &SDL_Metal_DestroyView);
        if (!metalView_)
        {
            std::cerr << "SDL_Metal_CreateView failed" << SDL_GetError() << '\n' << std::flush;
            return false;
        }
        pd.nwh = SDL_Metal_GetLayer(metalView_.get());
#endif


#ifdef SDL_PLATFORM_EMSCRIPTEN
        // For web, bgfx expects a canvas selector string or nullptr for default canvas.
        // pd.nwh = (void*)"#canvas"; // if you use a custom canvas element ID
        pd.nwh = nullptr; // let bgfx use the default canvas
#endif

        return true;
    }
}
