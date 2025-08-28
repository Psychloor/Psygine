// SPDX-FileCopyrightText: 2025 Kevin Blomqvist
// SPDX-License-Identifier: MIT

#include "psygine/core/runtime.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "SDL3/SDL_metal.h"

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"

#include "psygine/core/time.hpp"
#include "psygine/debug/assert.hpp"

namespace
{
    const char* RendererName(const bgfx::RendererType::Enum t)
    {
        switch (t)
        {
            case bgfx::RendererType::Direct3D11: return "D3D11";
            case bgfx::RendererType::Direct3D12: return "D3D12";
            case bgfx::RendererType::Vulkan: return "Vulkan";
            case bgfx::RendererType::Metal: return "Metal";
            case bgfx::RendererType::OpenGL: return "OpenGL";
            case bgfx::RendererType::OpenGLES: return "OpenGLES";
            default: return "Unknown/Auto";
        }
    }
}

namespace psygine::core
{
    Runtime::Runtime(RuntimeConfig config) :
        config_{std::move(config)}
    {
        PSYGINE_ASSERT(config.maxUpdatesPerTick > 0, "maxUpdatesPerTick must be greater than 0");
    }

    Runtime::~Runtime()
    {
        // Reverse order of initialization
        if (initialized_)
        {
            bgfx::shutdown();
            initialized_ = false;
        }
        window_.reset();

        if (SDL_WasInit(0) != 0)
        {
            shutdownGamepad();
            SDL_Quit();
        }
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
        if (config_.transparentWindow)
        {
            sdlWindowFlags |= SDL_WINDOW_TRANSPARENT;
        }

        if (config_.graphicsApi == GraphicsApi::Vulkan)
        {
            sdlWindowFlags |= SDL_WINDOW_VULKAN;
        }
        else if (config_.graphicsApi == GraphicsApi::OpenGL)
        {
            sdlWindowFlags |= SDL_WINDOW_OPENGL;
        }

        if (config_.graphicsApi == GraphicsApi::Metal || config_.graphicsApi == GraphicsApi::Any)
        {
#if !defined(SDL_PLATFORM_MACOS) && !defined(SDL_PLATFORM_IOS) && !defined(SDL_PLATFORM_TVOS)
            std::cerr << "Metal API requested but not available on this platform!\n";
            return false;
#endif
            // ReSharper disable once CppDFAUnreachableCode
            sdlWindowFlags |= SDL_WINDOW_METAL;
        }

        window_ = sdl_raii::CreateWindow(config_.title.c_str(), config_.width, config_.height, sdlWindowFlags);
        if (!window_)
        {
            std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n' << std::flush;
            SDL_Quit();
            return false;
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

        int pxW = 0, pxH = 0;
        SDL_GetWindowSizeInPixels(window_.get(), &pxW, &pxH);
        init.resolution.width = static_cast<uint32_t>(pxW);
        init.resolution.height = static_cast<uint32_t>(pxH);

        init.deviceId = config_.gpuDeviceId;
        init.resolution.reset = bgfxResetFlags();
        init.profile = config_.profile;
        init.debug = config_.debug;

        initialized_ = bgfx::init(init);
        if (!initialized_)
        {
            std::cerr << "bgfx::init failed: " << RendererName(bgfx::getRendererType()) << '\n' << std::flush;
            window_.reset();
            SDL_Quit();
            return false;
        }

        const bgfx::Caps* caps = bgfx::getCaps();
        std::cout << "Renderer: " << RendererName(caps->rendererType) << "\n";
        std::cout << "Max texture size: " << caps->limits.maxTextureSize << "\n";

        debug_ = config_.debug;
        wireframe_ = false;

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, config_.rgbaClearColor, 1.0F, 0);
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(pxW), static_cast<uint16_t>(pxH));

        return initialized_;
    }

    bool Runtime::initializeGamepad()
    {
        if ((SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) == SDL_INIT_GAMEPAD)
        {
            return true;
        }

        if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
        {
            std::cerr << "SDL_InitSubSystem failed: " << SDL_GetError() << '\n' << std::flush;
            return false;
        }
        return true;
    }

    void Runtime::shutdownGamepad()
    {
        if ((SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) == SDL_INIT_GAMEPAD)
        {
            SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
        }
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

        running_ = true;

        auto now = utils::time::Now();

        double accumulator = 0.0;
        std::size_t updatesThisFrame = 0;
        const double fixedTimestep = config_.fixedTimestep.count();
        const double maxTimestep = config_.maxTimestep.count();

        // for non-vsync
        constexpr double delayTimeStep = 1.0 / 240.0;

        while (running_)
        {
            handleEvents();

            // Protect some against lag spikes and all, kept within parentheses
            const double deltaTime = std::min(utils::time::ElapsedSinceSeconds(now), maxTimestep);
            now = utils::time::Now();
            lastDeltaTime_ = deltaTime;
            accumulator += deltaTime;

            updatesThisFrame = 0;
            while (accumulator >= fixedTimestep && updatesThisFrame < config_.maxUpdatesPerTick)
            {
                accumulator -= fixedTimestep;
                ++updatesThisFrame;
                fixedUpdate(fixedTimestep);
            }

            // Still too much lag, keep only the remainder
            if (accumulator >= fixedTimestep)
            {
                accumulator = std::fmod(accumulator, fixedTimestep);
            }

            update(deltaTime);

            const double interpolation = accumulator > 0.0 ? std::min(accumulator / fixedTimestep, 0.999999) : 0.0;
            render(interpolation);

            if (!config_.vsync && deltaTime < delayTimeStep)
            {
                SDL_DelayNS(1);
            }
        }
    }

    void Runtime::quit()
    {
        if (!running_)
        {
            return;
        }

        if (!onQuitRequested())
        {
            return;
        }

        running_ = false;
    }

    bool Runtime::isRunning() const
    {
        return running_;
    }

    bool Runtime::isInitialized() const
    {
        return initialized_;
    }

    bool Runtime::isGamepadInitialized()
    {
        return (SDL_WasInit(SDL_INIT_GAMEPAD) & SDL_INIT_GAMEPAD) == SDL_INIT_GAMEPAD;
    }

    const RuntimeConfig& Runtime::getConfig() const
    {
        return config_;
    }

    SDL_Window* Runtime::getWindow() const
    {
        return window_.get();
    }

    std::pair<int, int> Runtime::getBackBufferDimensions() const
    {
        int pxW = 0, pxH = 0;
        SDL_GetWindowSizeInPixels(window_.get(), &pxW, &pxH);
        return std::make_pair(pxW, pxH);
    }

    void Runtime::toggleDebug()
    {
        debug_ = !debug_;
        bgfx::setDebug(bgfxDebugFlags());
    }

    void Runtime::toggleWireframe()
    {
        wireframe_ = !wireframe_;
        bgfx::setDebug(bgfxDebugFlags());
    }

    void Runtime::set2DViewModeOrdering() const
    {
        if (!initialized_)
        {
            return;
        }

        // ReSharper disable once CppRedundantQualifierADL
        bgfx::setViewMode(0, bgfx::ViewMode::Sequential);
    }

    double Runtime::getCurrentFps() const
    {
        return 1.0 / lastDeltaTime_;
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
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                {
                    int pxW = 0, pxH = 0;
                    SDL_GetWindowSizeInPixels(window_.get(), &pxW, &pxH);
                    if (pxW > 0 && pxH > 0)
                    {
                        bgfx::reset(static_cast<uint32_t>(pxW), static_cast<uint32_t>(pxH), bgfxResetFlags());
                        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(pxW), static_cast<uint16_t>(pxH));
                    }
                    onEvent(event);
                    break;
                }

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

    std::uint32_t Runtime::bgfxDebugFlags() const
    {
        std::uint32_t flags = BGFX_DEBUG_NONE;
        if (wireframe_)
        {
            flags |= BGFX_DEBUG_WIREFRAME;
        }
        if (debug_)
        {
            flags |= BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS;
        }
        return flags;
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
        metalView_ = sdl_raii::CreateMetalView(window_.get());
        if (!metalView_)
        {
            std::cerr << "SDL_Metal_CreateView failed: " << SDL_GetError() << '\n' << std::flush;
            return false;
        }
        pd.nwh = SDL_Metal_GetLayer(metalView_.get());
#endif

        // iOS/tvOS (Metal)
#if defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_TVOS)
        metalView_ = sdl_raii::CreateMetalView(window_.get());
        if (!metalView_)
        {
            std::cerr << "SDL_Metal_CreateView failed: " << SDL_GetError() << '\n' << std::flush;
            return false;
        }
        pd.nwh = SDL_Metal_GetLayer(metalView_.get());
#endif

#ifdef SDL_PLATFORM_EMSCRIPTEN
        // For web, bgfx expects a canvas selector string or nullptr for default canvas.
        // pd.nwh = (void*)"#canvas"; // if you use a custom canvas element ID
        if (config_.customEmscriptenCanvas.empty())
        {
            pd.nwh = nullptr; // let bgfx use the default canvas
        }
        else
        {
            pd.nwh = (void*)config_.customEmscriptenCanvas.c_str();
        }
#endif

        return true;
    }
}
