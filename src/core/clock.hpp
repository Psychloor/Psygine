//
// Created by blomq on 2025-08-25.
//

#ifndef PSYGINE_CLOCK_HPP
#define PSYGINE_CLOCK_HPP

#include "core/time.hpp"

namespace psygine::core::utils::time
{
    /**
     * @brief Represents a Clock class for managing and tracking time.
     */
    class Clock
    {
    public:
        /**
         * @brief Constructs a new instance of the Clock class.
         *
         * Initializes the Clock to its start state with the current time as
         * its starting point. The clock is not running by default after
         * initialization.
         *
         * @return A new Clock object.
         */
        Clock();

        /**
         * @brief Default destructor for the Clock class.
         *
         * This destructor ensures proper cleanup of resources when
         * a Clock object goes out of scope. Since it is defined as
         * `= default`, the compiler will generate the implementation
         * automatically. The destructor performs no custom behavior.
         */
        ~Clock() = default;

        /**
         * @brief Starts the clock by resetting the start and end time points to the current time.
         *
         * If the clock is already running (as indicated by the `running` method), the call to this method
         * will have no effect.
         *
         * This method updates the start and end time points to the current time, effectively setting
         * the base time for measuring elapsed time.
         */
        void start();

        /**
         * Stops the clock if it is currently running.
         *
         * This method sets the end time of the clock to the current time, effectively
         * stopping the clock if it is in the running state. If the clock is already
         * stopped, the method does nothing.
         */
        void stop();

        /**
         * Restarts the clock by stopping it, calculating the elapsed time,
         * and then starting it again. This effectively resets the clock while
         * returning the duration elapsed since the last start or restart call.
         *
         * @return The duration that has elapsed since the last start or restart of the clock.
         */
        types::Duration restart();

        /**
         * Resets the clock to the current time.
         *
         * This method sets the internal start and end time points of
         * the clock to the current time, effectively restarting the
         * clock as though it has just been initialized.
         */
        void reset();

        /**
         * Calculates the elapsed time between the start and end time points.
         *
         * @return The duration representing the elapsed time.
         */
        [[nodiscard]] types::Duration elapsed() const;

        /**
         * Calculates the elapsed time in seconds between the start and end time points.
         *
         * @return The elapsed time in seconds as a double.
         */
        [[nodiscard]] double elapsedSeconds() const;

        /**
         * Returns the elapsed time in milliseconds between the start and end points of the clock.
         * The value is derived from the internally tracked start and end time points.
         *
         * @return The elapsed time in milliseconds as a double.
         */
        [[nodiscard]] double elapsedMilliseconds() const;

        /**
         * Checks whether the clock is currently running.
         * The clock is considered to be running if it has been started and not yet stopped.
         *
         * @return true if the clock is running, false otherwise.
         */
        [[nodiscard]] bool running() const;

        /**
         * Default copy constructor for the Clock class.
         * It performs a member-wise copy of the Clock object.
         *
         * @param other The Clock instance to copy from.
         * @return A new Clock instance that is a copy of the given instance.
         */
        Clock(const Clock& other) = default;

        /**
         * Move constructor for the Clock class.
         *
         * Transfers the contents of another Clock instance to this instance.
         * The source Clock instance is left in a valid but unspecified state.
         *
         * @param other The Clock instance to be moved from.
         * @return A new Clock instance with the data moved from the source instance.
         */
        Clock(Clock&& other) noexcept;

        /**
         * @brief Overloads the operator to define custom behavior for a specific operation.
         *
         * This operator function allows defining how the operator behaves when applied to instances
         * of this class or between this class and other types.
         *
         * @param other The right-hand side operand of the operator.
         * @return The result of the operation, its type depending on the definition of the operator.
         */
        Clock& operator=(const Clock& other);

        /**
         * @brief Overloaded operator for the class.
         * @param other The object to be used in the operator operation.
         * @return The result of the operator operation.
         */
        Clock& operator=(Clock&& other) noexcept;

    private:
        types::TimePoint start_;
        types::TimePoint end_;
    };
}

#endif //PSYGINE_CLOCK_HPP
