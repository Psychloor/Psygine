// SPDX-FileCopyrightText: 2025 Kevin Blomqvist
// SPDX-License-Identifier: MIT

#ifndef PSYGINE_TIME_HPP
#define PSYGINE_TIME_HPP

#include <chrono>

namespace psygine::core::utils::time
{
    namespace types
    {
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;
        using Duration = Clock::duration;
    }

    /**
     * Retrieves the current point in time as a `types::TimePoint`.
     *
     * This function uses the high-resolution clock defined in `types::Clock`
     * to fetch the current time. The returned value can be used for measuring
     * elapsed time or other time-related calculations.
     *
     * @return The current time as a `types::TimePoint`.
     */
    types::TimePoint Now();

    /**
     * Calculates the time duration between two time points.
     *
     * @param start The starting time point.
     * @param end The ending time point.
     * @return The duration between the two time points.
     */
    types::Duration Elapsed(types::TimePoint start, types::TimePoint end);

    /**
     * @brief Calculates the elapsed time in seconds between two time points.
     *
     * @param start The starting time point.
     * @param end The ending time point.
     * @return The elapsed time between the start and end time points, represented as a double in seconds.
     */
    double ElapsedSeconds(types::TimePoint start, types::TimePoint end);

    /**
     * @brief Computes the elapsed time in milliseconds between two time points.
     *
     * This function calculates the duration between the given start and end time points
     * and converts it to milliseconds as a double-precision floating-point value.
     *
     * @param start The start time point as a `types::TimePoint`.
     * @param end The end time point as a `types::TimePoint`.
     * @return The elapsed time in milliseconds as a double.
     */
    double ElapsedMilliseconds(types::TimePoint start, types::TimePoint end);

    /**
     * Calculates the elapsed time in microseconds between two time points.
     *
     * @param start The starting time point.
     * @param end The ending time point.
     * @return The elapsed time in microseconds as a double.
     */
    double ElapsedMicroseconds(types::TimePoint start, types::TimePoint end);

    /**
     * Calculates the elapsed time in nanoseconds between two time points.
     *
     * @param start The starting time point.
     * @param end The ending time point.
     * @return The elapsed time in nanoseconds as a double.
     */
    double ElapsedNanoseconds(types::TimePoint start, types::TimePoint end);

    /**
     * Calculates the duration of time that has elapsed since the provided starting time point.
     *
     * @param start The starting time point used as the reference for elapsed time measurement.
     * @return A duration representing the time elapsed from the provided starting point to the current time*/
    types::Duration ElapsedSince(types::TimePoint start);

    /**
     * Computes the time elapsed, in seconds, since the specified starting time point.
     *
     * This function calculates the duration, in seconds, between the provided starting
     * point and the current time point. It is particularly useful for measuring the
     * passage of time during the execution of a program or between specific events.
     *
     * @param start The starting time point from which elapsed time is measured.
     * @return The elapsed time in seconds as a double.
     */
    double ElapsedSinceSeconds(types::TimePoint start);

    /**
     * Calculates the elapsed time in milliseconds between the provided start time
     * and the current system time.
     *
     * This function retrieves the current system time, computes the duration
     * from the provided start time to the current time, and returns the
     * elapsed time in milliseconds as a double.
     *
     * @param start The starting time point from which the elapsed time is measured.
     * @return The elapsed time in milliseconds as a double.
     */
    double ElapsedSinceMilliseconds(types::TimePoint start);

    /**
     * Calculates the elapsed time in microseconds between the provided starting
     * time point and the current time.
     *
     * @param start The starting time point.
     * @return The elapsed time in microseconds as a double.
     */
    double ElapsedSinceMicroseconds(types::TimePoint start);

    /**
     * Computes the time elapsed in nanoseconds since the specified start time.
     *
     * This function calculates the difference in nanoseconds between the
     * provided start time and the current time, using the high-resolution clock.
     *
     * @param start The starting time point to calculate the duration from.
     * @return The elapsed time in nanoseconds as a double.
     */
    double ElapsedSinceNanoseconds(types::TimePoint start);
}

#endif //PSYGINE_TIME_HPP
