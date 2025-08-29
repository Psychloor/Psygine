//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

#ifndef PSYGINE_RUNTIME_CONFIG_HPP
#define PSYGINE_RUNTIME_CONFIG_HPP

#include <chrono>
#include <cstdint>
#include <string>

#include "bgfx/bgfx.h"

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
}

#endif //PSYGINE_RUNTIME_CONFIG_HPP