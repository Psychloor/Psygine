//
// Created by blomq on 2025-08-25.
//

#ifndef PSYGINE_RUNTIME_HPP
#define PSYGINE_RUNTIME_HPP

#include <chrono>
#include <string>

#include "sdl_raii.hpp"
#include "SDL3/SDL.h"
#include "bgfx/bgfx.h"

namespace psygine::core
{
    enum class GraphicsApi : std::uint8_t
    {
        None       = bgfx::RendererType::Noop,
        Acg        = bgfx::RendererType::Agc,
        Direct3D11 = bgfx::RendererType::Direct3D11,
        Direct3D12 = bgfx::RendererType::Direct3D12,
        Metal      = bgfx::RendererType::Metal,
        Nvn        = bgfx::RendererType::Nvn,
        OpenGL     = bgfx::RendererType::OpenGL,
        Vulkan     = bgfx::RendererType::Vulkan,
        OpenGLES   = bgfx::RendererType::OpenGLES,
        Gnm        = bgfx::RendererType::Gnm,
        Software   = bgfx::RendererType::Count,
        Any        = bgfx::RendererType::Count
    };

    enum class Msaa : std::uint32_t // NOLINT(*-enum-size) - bgfx is 32 bit unsigned int
    {
        None = BGFX_RESET_NONE,
        X2   = BGFX_RESET_MSAA_X2,
        X4   = BGFX_RESET_MSAA_X4,
        X8   = BGFX_RESET_MSAA_X8,
        X16  = BGFX_RESET_MSAA_X16,
    };

    struct RuntimeConfig
    {
        std::string title = "Psygine";
        std::uint16_t width = 1280;
        std::uint16_t height = 720;

        bool fullscreen = false;
        bool vsync = true;
        bool resizable = false;
        bool borderless = false;
        bool highDpi = false;
        bool hdr10 = false;

        bool debug = false;
        bool profiling = false;
        bool showCursor = true;

        std::size_t refreshRate = 60;
        std::chrono::duration<double> fixedTimestep = std::chrono::duration<double>(1 / 60.0);
        std::chrono::duration<double> maxTimestep = std::chrono::duration<double>(1);
        std::size_t maxUpdatesPerTick = 10;

        // If set to 0. will take the first good matching one
        std::uint16_t gpuDeviceId = 0;
        GraphicsApi graphicsApi = GraphicsApi::Any;
        Msaa msaa = Msaa::None;

        // In case there's anything specific not added here
        std::uint32_t bgfxCustomResetFlags = BGFX_RESET_NONE;
    };

    class Runtime
    {
    public:
        explicit Runtime(RuntimeConfig config);

        /**
         * @brief Destructor for the Runtime class.
         *
         * This destructor performs the cleanup and shutdown operations for the Runtime
         * instance. It handles the following tasks:
         * - Shuts down the bgfx subsystem.
         * - Releases the window resources.
         * - Deinitializes the gamepad subsystem if it was previously initialized.
         * - Cleans up the SDL subsystems.
         */
    protected:
        ~Runtime();

    public:
        bool initialize();
        bool initializeGamepad();
        void run();

        void setIsRunning(bool running);

        // Copy and Move Operations
        Runtime(const Runtime& other) = delete;
        Runtime(Runtime&& other) noexcept;
        Runtime& operator=(const Runtime& other) = delete;
        Runtime& operator=(Runtime&& other) noexcept;

    protected:
        virtual bool onQuitRequested();
        virtual void onEvent([[maybe_unused]] SDL_Event& event) = 0;
        virtual void onFixedUpdate([[maybe_unused]] double deltaTime) = 0;
        virtual void onUpdate([[maybe_unused]] double deltaTime) = 0;
        /**
         * bgfx::touch view0 has been called before and ends with bgfx::frame after this.
         * @param interpolation Interpolation between fixed frame ticks.
         */
        virtual void onRender([[maybe_unused]] double interpolation) = 0;

    private:
        void handleEvents();
        void fixedUpdate(double deltaTime);
        void update(double deltaTime);
        void render(double interpolation);

        [[nodiscard]] std::uint32_t bgfxResetFlags() const;
        void populatePlatformData(bgfx::PlatformData& pd);

        bool initialized_ = false;
        bool running_ = false;

        bool initializedGamepad_ = false;

        SdlWindowPtr window_{nullptr, &SDL_DestroyWindow};
        RuntimeConfig config_;
    };
}

#endif //PSYGINE_RUNTIME_HPP
