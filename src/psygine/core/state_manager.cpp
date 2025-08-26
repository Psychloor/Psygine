//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

//
// Created by blomq on 2025-08-26.
//

#include "psygine/core/state_manager.hpp"

#include <cassert>

#include "psygine/debug/assert.hpp"

namespace psygine::core::state
{
    void StateManager::push(StatePtr state, const LayerFlags flags)
    {
        PSYGINE_ASSERT(state != nullptr, "push: state must be non-null");
        pending_.push_back(PendingOp{.kind = OpKind::Push, .state = std::move(state), .flags = flags});
    }

    void StateManager::replaceTop(StatePtr state, const LayerFlags flags)
    {
        PSYGINE_ASSERT(state != nullptr, "push: state must be non-null");
        pending_.push_back(PendingOp{.kind = OpKind::ReplaceTop, .state = std::move(state), .flags = flags});
    }

    void StateManager::pop()
    {
        pending_.push_back(PendingOp{.kind = OpKind::Pop, .state = nullptr, .flags = LayerFlags{}});
    }

    void StateManager::clear()
    {
        pending_.push_back(PendingOp{.kind = OpKind::Clear, .state = nullptr, .flags = LayerFlags{}});
    }

    void StateManager::onEnterFrame()
    {
        if (!iterating_)
        {
            applyPending();
        }
    }

    void StateManager::onExitFrame()
    {
        if (!iterating_)
        {
            applyPending();
        }
    }

    bool StateManager::onQuitRequested()
    {
        if (layers_.empty())
        {
            return true;
        }

        iterating_ = true;
        bool allow = true;
        const std::size_t start = updateStartIndex();
        for (std::size_t i = layers_.size(); i-- > start;)
        {
            if (!layers_[i].state->onQuitRequested())
            {
                allow = false;
                break;
            }
        }
        iterating_ = false;
        return allow;
    }

    void StateManager::onEvent(SDL_Event& e)
    {
        if (layers_.empty())
        {
            return;
        }

        iterating_ = true;
        for (std::size_t i = layers_.size(); i-- > 0;)
        {
            layers_[i].state->onEvent(e);
            if (layers_[i].flags.modal)
            {
                break;
            }
        }
        iterating_ = false;
    }

    void StateManager::onFixedUpdate(const double dt)
    {
        if (layers_.empty())
        {
            return;
        }

        iterating_ = true;
        const std::size_t start = updateStartIndex();
        for (std::size_t i = start; i < layers_.size(); ++i)
        {
            layers_[i].state->onFixedUpdate(dt);
        }
        iterating_ = false;
    }

    void StateManager::onUpdate(const double dt)
    {
        if (layers_.empty())
        {
            return;
        }

        iterating_ = true;
        const std::size_t start = updateStartIndex();
        for (std::size_t i = start; i < layers_.size(); ++i)
        {
            layers_[i].state->onUpdate(dt);
        }
        iterating_ = false;
    }

    void StateManager::onRender(const double alpha)
    {
        if (layers_.empty())
        {
            return;
        }

        iterating_ = true;
        const std::size_t start = renderStartIndex();
        for (std::size_t i = start; i < layers_.size(); ++i)
        {
            layers_[i].state->onRender(alpha);
        }
        iterating_ = false;
    }

    std::size_t StateManager::topmostModalIndex() const
    {
        for (std::size_t i = layers_.size(); i-- > 0;)
        {
            if (layers_[i].flags.modal)
            {
                return i;
            }
        }
        return NPOS;
    }

    std::size_t StateManager::renderStartIndex() const
    {
        const std::size_t modalIdx = topmostModalIndex();
        if (modalIdx != NPOS)
        {
            if (!layers_[modalIdx].flags.allowRenderBelow)
            {
                return modalIdx;
            }
            return 0; // show what’s beneath too
        }
        return layers_.size() - 1; // render only top by default
    }

    std::size_t StateManager::updateStartIndex() const
    {
        const std::size_t modalIdx = topmostModalIndex();
        if (modalIdx != NPOS)
        {
            return modalIdx;
        }
        return layers_.size() - 1; // update only top by default
    }

    void StateManager::applyPending()
    {
        for (auto& op : pending_)
        {
            switch (op.kind)
            {
                case OpKind::Push:
                {
                    Layer layer{.state = std::move(op.state), .flags = op.flags};
                    layer.state->onEnter();
                    layers_.push_back(std::move(layer));
                }
                break;
                case OpKind::ReplaceTop:
                {
                    if (!layers_.empty())
                    {
                        layers_.back().state->onExit();
                        layers_.pop_back();
                    }
                    Layer layer{.state = std::move(op.state), .flags = op.flags};
                    layer.state->onEnter();
                    layers_.push_back(std::move(layer));
                }
                break;
                case OpKind::Pop:
                {
                    if (!layers_.empty())
                    {
                        layers_.back().state->onExit();
                        layers_.pop_back();
                    }
                }
                break;
                case OpKind::Clear:
                {
                    for (auto it = layers_.rbegin(); it != layers_.rend(); ++it)
                    {
                        it->state->onExit();
                    }
                    layers_.clear();
                }
                break;
            }
        }
        pending_.clear();
    }
}
