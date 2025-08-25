//
// Created by blomq on 2025-08-25.
//

#include "clock.hpp"

namespace psygine::core::utils::time
{

    Clock::Clock() : start_{Now()}, end_(start_)
    {}

    void Clock::start()
    {
        if (running_) {
            return;
        }

        start_ = Now();
        end_ = start_;
        running_ = true;
    }

    void Clock::stop()
    {
        if (!running_) {
            return;
        }

        end_ = Now();
        running_ = false;
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
        running_ = false;
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
        return running_;
    }

    Clock::Clock(Clock&& other) noexcept:
        running_(other.running_),
        start_(other.start_),
        end_(other.end_)
    {}

    Clock& Clock::operator=(const Clock& other)
    {
        if (this == &other)
        {
            return *this;
        }
        running_ = other.running_;
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
        running_ = other.running_;
        start_ = other.start_;
        end_ = other.end_;
        return *this;
    }
}
