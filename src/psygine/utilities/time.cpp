//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2025 Kevin Blomqvist
// SPDX-License-Identifier: MIT

#include "time.hpp"

namespace psygine::utilities::time
{

    types::TimePoint Now()
    {
        return types::Clock::now();
    }

    types::Duration Elapsed(const types::TimePoint start, const types::TimePoint end)
    {
        return end - start;
    }

    double ElapsedSeconds(const types::TimePoint start, const types::TimePoint end)
    {
        return std::chrono::duration<double>(end - start).count();
    }

    double ElapsedMilliseconds(const types::TimePoint start, const types::TimePoint end)
    {
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    double ElapsedMicroseconds(const types::TimePoint start, const types::TimePoint end)
    {
        return std::chrono::duration<double, std::micro>(end - start).count();
    }

    double ElapsedNanoseconds(const types::TimePoint start, const types::TimePoint end)
    {
        return std::chrono::duration<double, std::nano>(end - start).count();
    }

    types::Duration ElapsedSince(const types::TimePoint start)
    {
        return Elapsed(start, Now());
    }

    double ElapsedSinceSeconds(const types::TimePoint start)
    {
        return ElapsedSeconds(start, Now());
    }

    double ElapsedSinceMilliseconds(const types::TimePoint start)
    {
        return ElapsedMilliseconds(start, Now());
    }

    double ElapsedSinceMicroseconds(const types::TimePoint start)
    {
        return ElapsedMicroseconds(start, Now());
    }

    double ElapsedSinceNanoseconds(const types::TimePoint start)
    {
        return ElapsedNanoseconds(start, Now());
    }

}
