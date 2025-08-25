//
// Created by blomq on 2025-08-25.
//

#ifndef PSYGINE_TIME_HPP
#define PSYGINE_TIME_HPP

#include <chrono>

namespace psygine::core::utils::time
{
    namespace types
    {
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;
        using Duration = Clock::duration;
    }

    types::TimePoint Now();

    types::Duration Elapsed(types::TimePoint start, types::TimePoint end);
    double ElapsedSeconds(types::TimePoint start, types::TimePoint end);
    double ElapsedMilliseconds(types::TimePoint start, types::TimePoint end);
    double ElapsedMicroseconds(types::TimePoint start, types::TimePoint end);
    double ElapsedNanoseconds(types::TimePoint start, types::TimePoint end);

    types::Duration ElapsedSince(types::TimePoint start);
    double ElapsedSinceSeconds(types::TimePoint start);
    double ElapsedSinceMilliseconds(types::TimePoint start);
    double ElapsedSinceMicroseconds(types::TimePoint start);
    double ElapsedSinceNanoseconds(types::TimePoint start);
}

#endif //PSYGINE_TIME_HPP