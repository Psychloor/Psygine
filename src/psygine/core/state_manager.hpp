//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

#ifndef PSYGINE_STATE_MANAGER_HPP
#define PSYGINE_STATE_MANAGER_HPP

#include <memory>
#include <vector>

#include "psygine/core/base_state.hpp"

namespace psygine::core::state
{
    /**
     * @brief Manages a stack-based state system with support for layered states,
     * including modal and overlay behavior.
     *
     * The StateManager class provides functionality to manage a stack of states
     * in an application. It supports pushing, replacing, and removing states,
     * as well as handling runtime lifecycle callbacks such as events, updates,
     * and rendering. States can be stacked in layers, with options for managing
     * modal behavior and rendering beneath overlays.
     */
    class StateManager
    {
    public:
        using StatePtr = std::unique_ptr<BaseState>;

        struct LayerFlags
        {
            // If true, blocks events and updates from propagating to lower layers.
            bool modal = false;
            // If true and this is the topmost modal, render the layer(s) below before rendering this state.
            bool allowRenderBelow = false;

            explicit LayerFlags(const bool modal = false, const bool allowRenderBelow = false) noexcept :
                modal(modal), allowRenderBelow(allowRenderBelow)
            {}
        };

        StateManager() = default;
        ~StateManager() = default;

        // Non-copyable, movable
        StateManager(const StateManager&) = delete;
        StateManager& operator=(const StateManager&) = delete;
        StateManager(StateManager&&) noexcept = default;
        StateManager& operator=(StateManager&&) noexcept = default;

        /**
         * @brief Pushes a new state onto the state stack with specified layer behavior.
         *
         * The new state is added to the stack and configured with the provided layer flags,
         * which determine its interaction with events, updates, and rendering. This operation
         * is queued and applied at the appropriate time during the state manager's lifecycle.
         *
         * @param state A unique pointer to the state to be pushed. The state must not be null.
         * @param flags Configuration flags for the state's layer behavior, such as modal or overlay settings.
         */
        void push(StatePtr state, LayerFlags flags = LayerFlags());

        /**
         * @brief Pushes a new modal state onto the state stack, optionally allowing rendering of the layers below.
         *
         * Adds a new state configured as modal to the state stack. Modal states block events
         * and updates to lower layers by default. By specifying the `allowRenderBelow` parameter,
         * it is possible to determine whether layers below the modal state should be rendered.
         *
         * @param state A unique pointer to the state to be pushed as a modal. The state must not be null.
         * @param allowRenderBelow If true, the layers below this modal state will remain visible during rendering;
         *                         otherwise, they will be hidden. Defaults to true.
         */
        void pushModal(StatePtr state, const bool allowRenderBelow = true)
        {
            push(std::move(state), LayerFlags{true, allowRenderBelow});
        }

        // Replace the top state with a new one (keeps stack depth).
        void replaceTop(StatePtr state, LayerFlags flags = LayerFlags());
        // Pop the top state.
        void pop();
        // Clear all states.
        void clear();

        // Frame lifecycle – call these from your app loop.
        void onEnterFrame(); // applies pending ops before frame begins (optional)
        void onExitFrame();  // applies pending ops after frame ends (recommended)

        // Forward Runtime callbacks into the stack:
        bool onQuitRequested(); // returns false if the active segment vetoes quit
        void onEvent(SDL_Event& e);
        void onFixedUpdate(double dt);
        void onUpdate(double dt);
        void onRender(double alpha);

        // Introspection
        [[nodiscard]] bool empty() const
        {
            return layers_.empty();
        }

        [[nodiscard]] std::size_t size() const
        {
            return layers_.size();
        }

    private:
        struct Layer
        {
            StatePtr state;
            LayerFlags flags;
        };

        enum class OpKind : std::uint8_t { Push, ReplaceTop, Pop, Clear };

        struct PendingOp
        {
            OpKind kind;
            StatePtr state;
            LayerFlags flags;
        };

        // Returns index of the topmost modal layer, or npos if none.
        [[nodiscard]] std::size_t topmostModalIndex() const;
        // Determine the lowest index to start rendering from (bottom of the visible segment).
        [[nodiscard]] std::size_t renderStartIndex() const;
        // Determine the first index to update/event from (topmost modal if present; else top only).
        [[nodiscard]] std::size_t updateStartIndex() const;

        void applyPending(); // commit queued structural changes

        static constexpr std::size_t NPOS = static_cast<std::size_t>(-1);

        std::vector<Layer> layers_;
        std::vector<PendingOp> pending_;
        bool iterating_ = false; // guard against direct mutation during callbacks
    };
}


#endif //PSYGINE_STATE_MANAGER_HPP
