// SPDX-FileCopyrightText: 2025 Kevin Blomqvist
// SPDX-License-Identifier: MIT

#ifndef PSYGINE_RUNTIME_HPP
#define PSYGINE_RUNTIME_HPP

#include <chrono>
#include <string>

#include "SDL3/SDL.h"
#include "bgfx/bgfx.h"
#include "psygine/core/sdl_raii.hpp"

namespace psygine::core
{

    /**
     * @brief Enumeration of graphics APIs supported by the rendering engine.
     *
     * This enum defines a set of graphics API backends that may be used for rendering purposes.
     * Each member corresponds to a specific rendering API or graphics processing unit (GPU) type.
     * The selection of a graphics API determines the underlying rendering technology employed
     * during runtime.
     *
     * - `None`: No rendering is performed.
     * - `Acg`: AGC rendering API.
     * - `Direct3D11`: Microsoft Direct3D 11 rendering API.
     * - `Direct3D12`: Microsoft Direct3D 12 rendering API.
     * - `Metal`: Apple Metal rendering API.
     * - `Nvn`: NVN rendering API for Nintendo hardware.
     * - `OpenGL`: OpenGL (version 2.1+).
     * - `Vulkan`: Vulkan rendering API for modern GPUs.
     * - `OpenGLES`: OpenGL ES (version 2.0+), commonly used on mobile and embedded devices.
     * - `Gnm`: GNM rendering API for Sony PlayStation platforms.
     * - `Software`: Special mode using software-level rendering. Often used for compatibility.
     * - `Any`: The engine will automatically select an appropriate API.
     */
    // ReSharper disable CppInconsistentNaming
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

    // ReSharper restore CppInconsistentNaming

    /**
     * @brief Enumeration for Multi-Sample Anti-Aliasing (MSAA) levels.
     *
     * This enum defines the available MSAA levels that can be applied to the rendering context
     * to improve visual quality by reducing aliasing effects. Each enum member corresponds
     * to a specific number of samples used for antialiasing. Higher values increase visual
     * quality at the cost of performance.
     *
     * - `None`: Disables Multi-Sample Anti-Aliasing.
     * - `X2`: Enables 2x Multi-Sample Anti-Aliasing.
     * - `X4`: Enables 4x Multi-Sample Anti-Aliasing.
     * - `X8`: Enables 8x Multi-Sample Anti-Aliasing.
     * - `X16`: Enables 16x Multi-Sample Anti-Aliasing.
     */
    enum class Msaa : std::uint32_t // NOLINT(*-enum-size) - bgfx is 32 bit unsigned int
    {
        None = BGFX_RESET_NONE,
        X2   = BGFX_RESET_MSAA_X2,
        X4   = BGFX_RESET_MSAA_X4,
        X8   = BGFX_RESET_MSAA_X8,
        X16  = BGFX_RESET_MSAA_X16,
    };

    /**
     * @brief Configuration structure for the runtime settings.
     *
     * This structure defines various configuration parameters for the runtime system.
     * These settings include screen resolution, graphical settings, timing details, and
     * other system configurations. Each field has a default value, ensuring a valid
     * initial state for typical use cases.
     *
     * The configuration options provide flexibility to tailor the behavior and appearance
     * of the runtime environment to meet specific requirements.
     *
     * - `title`: The title of the application window.
     * - `width` and `height`: Dimensions of the application window in pixels.
     * - `fullscreen`: Flag indicating whether the application runs in fullscreen mode.
     * - `vsync`: Enables or disables vertical synchronization (V-Sync).
     * - `resizable`: Determines if the application window is resizable.
     * - `borderless`: Indicates whether the window is borderless.
     * - `highDpi`: Enables High-DPI support for the window.
     * - `hdr10`: Enables HDR10 support if available.
     * - `transparentWindow`: Allows the window to have transparency.
     * - `debug`: Activates debug mode to provide additional debugging information.
     * - `fixedTimestep`: Sets the interval for fixed-update logic.
     * - `maxTimestep`: Specifies the maximum duration for catching up during updates.
     * - `maxUpdatesPerTick`: Defines the maximum number of update steps allowed per tick.
     * - `gpuDeviceId`: Specifies the GPU device ID to use; 0 selects the first compatible GPU.
     * - `graphicsApi`: Determines the graphics API to use (e.g., DirectX, Vulkan, OpenGL).
     * - `msaa`: Configures the level of Multi-Sample Anti-Aliasing (MSAA).
     * - `rgbaClearColor`: Sets the clear color for the rendering context in RGBA format.
     * - `bgfxCustomResetFlags`: Allows custom flags for BGFX reset settings.
     */
    struct RuntimeConfig
    {
        std::string title = "Psygine";
        std::uint16_t width = 1280;
        std::uint16_t height = 720;

        bool fullscreen = false;
        bool vsync = true;
        bool resizable = false;
        bool borderless = false;
        bool highDpi = true;
        bool hdr10 = false;
        bool transparentWindow = false;

        bool debug = false;
        bool profile = false;

        std::chrono::duration<double> fixedTimestep = std::chrono::duration<double>(1 / 60.0);
        std::chrono::duration<double> maxTimestep = std::chrono::duration<double>(1);
        std::size_t maxUpdatesPerTick = 10;

        // If set to 0. will take the first good matching one
        std::uint16_t gpuDeviceId = 0;
        GraphicsApi graphicsApi = GraphicsApi::Any;
        Msaa msaa = Msaa::None;

        // CornFlowerBlue - Update it if using a transparent window
        std::uint32_t rgbaClearColor = 0x93CCEAFF;

        // In case there's anything specific not added here
        std::uint32_t bgfxCustomResetFlags = BGFX_RESET_NONE;

        // "#canvas" if you use a custom canvas id/element
        std::string customEmscriptenCanvas;
    };

    class Runtime : public std::enable_shared_from_this<Runtime> // NOLINT(*-virtual-class-destructor)
    {
        /**
         * @brief Constructor for the Runtime class.
         *
         * This constructor initializes the Runtime instance with the specified configuration.
         * It ensures that the provided configuration is valid by asserting that
         * the maximum updates per tick are greater than zero. The configuration is
         * stored for later use.
         *
         * @param config The configuration parameters for the Runtime instance.
         *               It includes settings such as the maximum updates per tick.
         *
         * @throws std::logic_error If the provided `maxUpdatesPerTick` value is not greater than zero
         *                          during assertion in debug builds.
         */
    public:
        explicit Runtime(RuntimeConfig config);

    protected:
        /**
         * @brief Destructor for the Runtime class.
         *
         * Cleans up resources and performs necessary shutdown operations during the
         * destruction of a Runtime instance. It ensures proper termination
         * of systems initialized during the Runtime's lifetime.
         *
         * - Shuts down the rendering backend if it was initialized.
         * - Releases the allocated window resources.
         * - Tears down SDL systems if they were previously initialized.
         * - Handles gamepad subsystem shutdown, if applicable.
         *
         * This method ensures that all Runtime-associated resources are properly
         * released, preventing memory leaks or undefined behavior upon program exit.
         */
        virtual ~Runtime();

    public:
        /**
         * @brief Initializes the Runtime environment and its dependencies.
         *
         * This method performs the setup necessary for the Runtime instance to operate,
         * including
         * - Initializing the SDL subsystem with required flags.
         * - Creating an SDL window based on the specified configuration options such as
         *   high DPI, fullscreen mode, resizability, borderless mode, and transparency.
         * - Configuring bgfx with platform-specific data and renderer settings.
         * - Establishing a graphics context with the appropriate resolution and GPU configuration.
         *
         * @return True if the initialization succeeds; otherwise, false.
         */
        bool initialize();

        /**
         * @brief Initializes the gamepad subsystem.
         *
         * This method ensures that the gamepad subsystem is initialized and ready for use.
         * If the subsystem is already initialized, the method returns immediately. Otherwise,
         * an attempt is made to initialize the required gamepad support through the underlying
         * SDL library. Errors encountered during initialization will prevent the subsystem
         * from being activated.
         *
         * @return `true` if the gamepad subsystem is successfully initialized or already active.
         *         `false` if the initialization fails.
         */
        static bool initializeGamepad();

        /**
         * @brief Shuts down and cleans up the initialized gamepad subsystem.
         *
         * This method ensures that the gamepad subsystem, if previously initialized,
         * is properly shut down and any allocated resources are released.
         * It checks the initialization state of the gamepad subsystem before
         * attempting to terminate it, preventing unintended behavior.
         */
        static void shutdownGamepad();

        /**
         * @brief Executes the main runtime loop of the engine.
         *
         * This method runs the main loop of the runtime, handling the following tasks:
         * - Ensures the runtime is properly initialized and not already running.
         * - Configures debug settings and clears the rendering output.
         * - Processes incoming events through the event handling mechanism.
         * - Manages a fixed timestep for updates while ensuring that lag spikes are mitigated.
         * - Executes the fixed update and interpolation logic to maintain accurate simulation and rendering.
         * - Calls the update and render methods for per-frame updates and rendering operations.
         *
         * The loop continues running until the runtime is signaled to stop.
         *
         * @note If the runtime is not initialized or already running, appropriate error messages will be logged.
         */
        void run();

        /**
         * @brief Stops the application runtime and terminates the running state.
         *
         * This method is responsible for stopping the Runtime instance by setting its
         * running state to `false`, effectively terminating the application's main loop.
         * It performs the following steps:
         * - Checks if the runtime is currently running. If not, the method returns immediately.
         * - Calls the `onQuitRequested` method to determine if the quit request is approved.
         * - If `onQuitRequested` returns `true`, the runtime's running state is set to `false`,
         *   signaling the application to terminate.
         *
         * @note If the runtime is not already running or the `onQuitRequested` callback
         * denies the quit request, the method returns without making changes.
         */
        void quit();

        /**
         * @brief Checks whether the Runtime is currently running.
         *
         * This method verifies the current state of the Runtime instance to determine
         * if it is actively running.
         *
         * @return True if the Runtime is running; otherwise, false.
         */
        [[nodiscard]] bool isRunning() const;

        /**
         * @brief Checks if the Runtime has been initialized.
         *
         * This method determines whether the Runtime instance has completed its
         * initialization process successfully.
         *
         * @return True if the Runtime is initialized, false otherwise.
         */
        [[nodiscard]] bool isInitialized() const;

        /**
         * @brief Checks whether the gamepad subsystem has been initialized.
         *
         * This method returns the status of the gamepad subsystem initialization.
         * It provides a way to verify if the gamepad functionality is ready to use.
         *
         * @return True if the gamepad subsystem is initialized, false otherwise.
         */
        [[nodiscard]] static bool isGamepadInitialized();

        /**
         * @brief Retrieves the runtime configuration.
         *
         * This method provides access to the configuration settings for the runtime
         * instance. It allows the retrieval of various runtime-specific parameters and
         * options stored in the RuntimeConfig structure.
         *
         * @return A constant reference to the RuntimeConfig object associated with the runtime instance.
         */
        [[nodiscard]] const RuntimeConfig& getConfig() const;

        /**
         * @brief Retrieves the native SDL_Window instance associated with the runtime.
         *
         * This method provides access to the SDL_Window object that is managed by the
         * Runtime class. The returned window can be used for further window-specific
         * operations or configurations.
         *
         * @return A pointer to the managed SDL_Window instance.
         */
        [[nodiscard]] SDL_Window* getWindow() const;

        /**
         * @brief Retrieves the dimensions of the back buffer in pixels.
         *
         * This method queries the width and height of the current rendering surface (back buffer).
         * The size returned reflects the actual pixel dimensions of the rendering window, which may
         * differ from logical window size due to scaling or high-DPI settings.
         *
         * @return A pair of integers where the first element is the width in pixels and the second
         * element is the height in pixels.
         */
        [[nodiscard]] std::pair<int, int> getBackBufferDimensions() const;

        /**
         * @brief Toggles the debug mode of the runtime environment.
         *
         * This method switches the runtime's debug state between enabled and disabled.
         * When debug mode is enabled, additional debugging information or features
         * may be activated, as controlled by the rendering subsystem.
         *
         * Internally, this method updates the debug state flag and configures
         * the rendering engine's debug settings to reflect the new state.
         */
        void toggleDebug();

        /**
         * @brief Toggles the wireframe rendering mode for the application.
         *
         * This function switches the rendering mode between the wireframe and the default mode.
         * When invoked, it updates the internal wireframe state and adjusts the rendering
         * debug settings accordingly.
         *
         * Wireframe mode is commonly used for debugging or visualizing the geometric structure
         * of rendered 3D models by displaying their wireframe outlines instead of fully shaded surfaces.
         */
        void toggleWireframe();

        /**
         * @brief Configures the 2D view mode ordering for rendering.
         *
         * This method sets the 2D view mode for a rendering system to Sequential.
         * It ensures that the view ordering follows a sequential rendering order,
         * which can be important for specific rendering pipelines or scenarios.
         * The method performs this configuration only if the runtime is properly initialized.
         *
         * If the runtime is not initialized, the method terminates without making changes.
         */
        void set2DViewModeOrdering() const;

        [[nodiscard]] double getCurrentFps() const;

        // Copy and Move Operations
        Runtime(const Runtime& other) = delete;
        Runtime(Runtime&& other) noexcept = delete;
        Runtime& operator=(const Runtime& other) = delete;
        Runtime& operator=(Runtime&& other) noexcept = delete;

    protected:
        /**
         * @brief Handles quit requests from the application, such as when the user
         * attempts to close the window.
         *
         * This method is invoked when a quit request event (e.g., SDL_EVENT_QUIT)
         * is received. It allows the program to perform cleanup or confirmation
         * before allowing the application to close.
         *
         * @return A boolean value indicating whether the quit request is approved.
         * Returning `true` proceeds with the application's termination. Returning
         * `false` cancels the quit process, keeping the application running.
         */
        virtual bool onQuitRequested();

        /**
         * @brief Handles SDL events.
         *
         * This pure virtual method is invoked whenever an SDL event occurs.
         * Derived classes must implement this function to define their specific
         * behavior for handling SDL events.
         *
         * @param event A reference to the SDL_Event object containing event details.
         */
        virtual void onEvent([[maybe_unused]] SDL_Event& event) = 0;

        /**
         * @brief Called during the fixed update loop to handle physics or other fixed-timestep updates.
         *
         * This pure virtual method is invoked at regular time intervals, providing a stable
         * update rate. Derived classes must implement this function to define logic that needs
         * to be executed on a fixed timestep, such as physics or other deterministic updates.
         *
         * @param deltaTime The fixed time interval in seconds between updates.
         */
        virtual void onFixedUpdate([[maybe_unused]] double deltaTime) = 0;

        /**
         * @brief Called to update state with the elapsed time since the last update.
         *
         * This pure virtual method is invoked during the game loop to update the
         * state of the application. Derived classes must implement this function
         * to define their specific update logic.
         *
         * @param deltaTime The time, in seconds, that has elapsed since the last update call. Unclamped
         */
        virtual void onUpdate([[maybe_unused]] double deltaTime) = 0;

        /**
         * @brief Renders the content during each frame update.
         *
         * This pure virtual method is invoked during the rendering phase of the application's
         * game loop. Derived classes must implement this function to define their specific
         * rendering logic. It should be used to draw objects, UI components, and other
         * graphical elements.
         *
         * @param interpolation The interpolation value (typically between 0 and 1) used for
         * smoothing animations or rendering frames between discrete updates.
         */
        virtual void onRender([[maybe_unused]] double interpolation) = 0;

    private:
        void handleEvents();
        void fixedUpdate(double deltaTime);
        void update(double deltaTime);
        void render(double interpolation);

        [[nodiscard]] std::uint32_t bgfxDebugFlags() const;
        [[nodiscard]] std::uint32_t bgfxResetFlags() const;
        bool populatePlatformData(bgfx::PlatformData& pd);

        bool initialized_ = false;
        bool running_ = false;
        double lastDeltaTime_ = 0.0;

        bool debug_ = false;
        bool wireframe_ = false;

        SdlWindowPtr window_{nullptr, &SDL_DestroyWindow};
        SdlMetalViewPtr metalView_{nullptr, &SDL_Metal_DestroyView};
        RuntimeConfig config_;
    };
}

#endif //PSYGINE_RUNTIME_HPP
