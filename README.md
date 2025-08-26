# Psygine

Psygine is a small C++23 runtime scaffold built on SDL3 and bgfx. 
It aims to provide a clean, portable foundation for high-performance, 
multithreaded rendering with a “Fix Your Timestep” main loop, 
renderer-agnostic shader flow, and minimal platform glue.

- Rendering backends: Direct3D 11/12, Vulkan, Metal, OpenGL/OpenGLES, and more (via bgfx)
- Windowing, input, and platform integration via SDL3
- Deterministic fixed-update loop with interpolation
- Optional VSYNC, MSAA, HDR flags passed to bgfx
- High-DPI aware, resizable, exclusive/borderless fullscreen modes
- Cross-platform: Windows, Linux (X11/Wayland), macOS/iOS/tvOS (Metal), Android, Web (Emscripten)

## Features

- Renderer-agnostic: select your preferred graphics API at runtime
- Clean separation of update (fixed/variable) and render
- Platform data setup for bgfx via SDL3 native handles (including Metal layers)
- Simple configuration struct for common flags (vsync, msaa, dpi, hdr, etc.)
- Straightforward event handling passthrough

## Requirements

- C++23 toolchain (MSVC, Clang, or GCC)
- CMake 4.0+ recommended
- SDL3 (CONFIG package or development files)
- bgfx (fetched via bgfx.cmake or supplied by your environment)

## Building

Using CMake:

```shell script
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

Options you can pass to CMake:
- ENABLE_WARNINGS_AS_ERRORS=ON|OFF
- ENABLE_UNITY=ON|OFF
- ENABLE_SANITIZERS=ON|OFF (Clang/GCC only)

If SDL3 is installed via a package manager that provides a CMake config, it should be detected automatically. 
Otherwise, set SDL3_DIR to the location of its config files.

## Quick Start

Create a minimal application by deriving from the runtime and implementing the hooks:

```cpp
#include <memory>
#include "psygine/core/runtime.hpp"

class App : public psygine::core::Runtime {
public:
    using Runtime::Runtime;

private:
    void onEvent(SDL_Event& e) override {
        if (e.type == SDL_EVENT_KEY_DOWN /* ... */) {
            // handle input
        }
    }

    void onFixedUpdate(double dt) override {
        // deterministic logic (physics, simulation), dt = config.fixedTimestep
    }

    void onUpdate(double dt) override {
        // variable-step logic (animations, UI)
    }

    void onRender(double alpha) override {
        // issue bgfx view commands; alpha is interpolation factor [0..1)
        // bgfx::touch(0) is called before, bgfx::frame() is called after
    }
};

int main() {
    psygine::core::RuntimeConfig cfg;
    cfg.title = "Psygine Demo";
    cfg.width = 1280;
    cfg.height = 720;
    cfg.graphicsApi = psygine::core::GraphicsApi::Any; // let bgfx pick, or force one
    cfg.vsync = true;
    cfg.msaa = psygine::core::Msaa::X4;

    auto app = std::make_shared<App>(cfg);
    if (!app->initialize()) {
        return 1;
    }
    app->run();
    return 0;
}
```


## Main Loop Philosophy

The runtime implements a “Fix Your Timestep” loop:
- Fixed update runs at a constant step (e.g., 1/60s) to keep simulation deterministic.
- Variable update gets the current frame’s dt for smooth, frame-rate-independent logic. BEWARE: variable updates delta-time aren't clamped.
- Interpolation factor is provided to render between fixed steps.
- Protections against the “spiral of death” cap catch-up work and trim excess lag.

This provides smooth rendering even if the simulation runs at a steady fixed rate.

## Configuration Highlights

- title, width, height: window properties
- fullscreen, borderless, resizable, highDpi, transparentWindow
- vsync: toggles sync to display refresh (through bgfx reset flags)
- msaa: MSAA level (None, x2, x4, x8, x16)
- hdr10: request HDR10 formats where supported
- graphicsApi: pick a renderer (D3D11/12, Vulkan, Metal, OpenGL/ES, etc.) or Any
- fixedTimestep: physics/simulation step (default 1/60)
- maxTimestep and maxUpdatesPerTick: guardrails for frame spikes
- rgbaClearColor: default clear color
- bgfxCustomResetFlags: pass-through for advanced bgfx reset bits
- customEmscriptenCanvas: override canvas element selector on Web

## Platform Notes

- Windows: currently defining NOMINMAX and WIN32_LEAN_AND_MEAN a long with passing an utf-8 flag.
- macOS/iOS/tvOS (Metal): a CAMetalLayer is attached to the SDL Metal view and passed to bgfx as the native handle. This ensures native Metal presentation.
- Linux: X11 and Wayland are supported via SDL3’s native handles.
- Android: the native window is passed via SDL3.
- Web (Emscripten): uses the default canvas unless you set a custom selector.

## VSync and Refresh Rate

- VSync: enable via the runtime configuration; frames are paced by the display’s refresh.
- Refresh rate: in windowed/borderless mode the monitor refresh cannot be forced. In exclusive fullscreen you can select a display mode with a specific refresh rate via SDL3, if supported by the OS/driver.

## Shaders

Bgfx uses platform-agnostic shaders compiled per-backend. You can author shaders in bgfx’s shader language and compile them for all targets, keeping your render code portable across APIs.

## Roadmap

- Optional frame pacing/capping utilities when VSync is off
- Hot-reload for shaders and pipelines
- Input/gamepad helpers
- Sample scenes and debug UI integrations

## Contributing

Issues and PRs are welcome. Please keep changes focused, with clear motivation and platform testing when possible.