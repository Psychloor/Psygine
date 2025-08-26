//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT


#ifndef PSYGINE_BASE_STATE_HPP
#define PSYGINE_BASE_STATE_HPP

// ReSharper disable once CppInconsistentNaming
union SDL_Event;

namespace psygine::core::state
{
    struct BaseState
    {
        virtual ~BaseState() = default;

        virtual void onEnter() = 0;
        virtual void onExit() = 0;

        [[nodiscard]] virtual bool onQuitRequested()
        {
            return true;
        }

        virtual void onEvent([[maybe_unused]] SDL_Event& event) = 0;
        virtual void onFixedUpdate([[maybe_unused]] double deltaTime) = 0;
        virtual void onUpdate([[maybe_unused]] double deltaTime) = 0;
        virtual void onRender([[maybe_unused]] double interpolation) = 0;

        // Delete Copy/Move
        BaseState(const BaseState& other) = default;
        BaseState(BaseState&& other) noexcept = delete;
        BaseState& operator=(const BaseState& other) = default;
        BaseState& operator=(BaseState&& other) noexcept = delete;
    };
}

#endif //PSYGINE_BASE_STATE_HPP
