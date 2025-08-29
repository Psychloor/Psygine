//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2025 Kevin Blomqvist
// SPDX-License-Identifier: MIT

#include "clock.hpp"

namespace psygine::utilities::time
{

    Clock::Clock() :
        start_{Now()}, end_(start_)
    {}

    void Clock::start()
    {
        if (running())
        {
            return;
        }

        start_ = Now();
        end_ = start_;
    }

    void Clock::stop()
    {
        if (!running())
        {
            return;
        }

        end_ = Now();
    }

    types::Duration Clock::restart()
    {
        stop();
        const auto elapsed = this->elapsed();
        start();
        return elapsed;
    }

    void Clock::reset()
    {
        start_ = Now();
        end_ = start_;
    }

    types::Duration Clock::elapsed() const
    {
        return Elapsed(start_, end_);
    }

    double Clock::elapsedSeconds() const
    {
        return ElapsedSeconds(start_, end_);
    }

    double Clock::elapsedMilliseconds() const
    {
        return ElapsedMilliseconds(start_, end_);
    }

    bool Clock::running() const
    {
        return start_ != end_;
    }

    Clock::Clock(Clock&& other) noexcept :
        start_(other.start_),
        end_(other.end_)
    {}

    Clock& Clock::operator=(const Clock& other)
    {
        if (this == &other)
        {
            return *this;
        }

        start_ = other.start_;
        end_ = other.end_;
        return *this;
    }

    Clock& Clock::operator=(Clock&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        start_ = other.start_;
        end_ = other.end_;
        return *this;
    }
}
