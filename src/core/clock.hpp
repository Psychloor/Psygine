//
// Created by blomq on 2025-08-25.
//

#ifndef PSYGINE_CLOCK_HPP
#define PSYGINE_CLOCK_HPP

#include "core/time.hpp"

namespace psygine::core::utils::time
{
    class Clock
    {
    public:
        Clock();
        ~Clock() = default;

        void start();
        void stop();
        types::Duration restart();
        void reset();

        [[nodiscard]] types::Duration elapsed() const;
        [[nodiscard]] double elapsedSeconds() const;
        [[nodiscard]] double elapsedMilliseconds() const;

        [[nodiscard]] bool running() const;

        Clock(const Clock& other) = default;
        Clock(Clock&& other) noexcept;
        Clock& operator=(const Clock& other);
        Clock& operator=(Clock&& other) noexcept;

    private:
        bool running_ = false;
        types::TimePoint start_;
        types::TimePoint end_;
    };
}

#endif //PSYGINE_CLOCK_HPP
