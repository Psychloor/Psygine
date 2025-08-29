//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

//  SPDX-FileCopyrightText: 2025 Kevin Blomqvist
//  SPDX-License-Identifier: MIT

#ifndef PSYGINE_RANDOM_HPP
#define PSYGINE_RANDOM_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <random>
#include <type_traits>
#include <vector>

namespace psygine::utilities::random
{
    namespace detail
    {
        /**
         * @brief Specialization of the HasStateSize trait for types that do not provide a `state_size` member.
         *
         * This struct is the default implementation of the HasStateSize trait and inherits from `std::false_type`.
         * It represents the absence of a `state_size` member in the specified type.
         *
         * @tparam T The type being checked for a `state_size` member.
         * @tparam U A placeholder template parameter for SFINAE defaults to `void`.
         */
        template <typename T, typename U = void>
        struct HasStateSize : std::false_type
        {};

        /**
         * @brief Trait to detect whether a type provides a `state_size` member.
         *
         * Specialization for types that define a `state_size` member. When a type `T` has a
         * `state_size` member, this specialization inherits from `std::true_type`, effectively
         * evaluating to true for trait checks.
         *
         * @tparam T The type to be checked for the presence of a `state_size` member.
         */
        template <typename T>
        struct HasStateSize<T, std::void_t<decltype(T::state_size)>> : std::true_type
        {};

        /**
         * @brief Determines if a given type has a `state_size` member at compile-time.
         *
         * This variable template evaluates to `true` if the type `T` has a `state_size` member,
         * as determined by the `HasStateSize` trait. Otherwise, it evaluates to `false`.
         *
         * @tparam T The type being checked for the presence of a `state_size` member.
         */
        template <typename T>
        inline constexpr bool HAS_STATE_SIZE_V = HasStateSize<T>::value;

        /**
        * @brief Concept that checks whether a random engine can be seeded via std::seed_seq.
        *
        * A type Engine models SeedSeqConstructibleOrSeedable if it supports seeding from a seed sequence
        * by either:
        * - being constructible from a std::seed_seq (Engine{seq}), or
        * - exposing a member function seed(std::seed_seq&).
        *
        * This concept is used to constrain helper functions that initialize RNG engines from high‑entropy
        * seed material gathered into a std::seed_seq.
        *
        * @tparam Engine Candidate RNG engine type.
        *
        * @note Standard engines such as std::mt19937 and std::mt19937_64 satisfy this concept.
        *
        * @code
        * #include <random>
        * static_assert(SeedSeqConstructibleOrSeedable<std::mt19937>);
        * static_assert(SeedSeqConstructibleOrSeedable<std::mt19937_64>);
        * @endcode
        */
        template <typename Engine>
        concept SeedSeqConstructibleOrSeedable = requires(std::seed_seq& s, Engine& e)
        {
            Engine{s}; // constructor from a seed sequence
            e.seed(s); // or ability to seed with a seed sequence
        };


        /**
         * @brief Computes and returns the number of 32-bit words to use for seeding a random number generator (RNG) engine.
         *
         * @tparam Engine The RNG engine type to consider. If the type has a `state_size` member,
         * its value is used. Otherwise, a default value of 16 (representing 512 bits of seed material) is returned.
         *
         * @return The number of 32-bit words required for seeding the specified Engine type. If `state_size` is
         * defined for the Engine, its value is used. In the absence of `state_size`, a default heuristic of 16 words
         * is applied to ensure sufficient diffusion of randomness while maintaining computational efficiency.
         */
        template <typename Engine>
        constexpr std::size_t SeedWordCount()
        {
            if constexpr (HAS_STATE_SIZE_V<Engine>)
            {
                return static_cast<std::size_t>(Engine::state_size);
            }
            else
            {
                // Heuristic: more than 1 to give good diffusion, but not huge.
                // 16 x 32-bit words = 512 bits of seed material.
                return 16;
            }
        }

        /**
         * @brief Creates a seed sequence for initializing random number generators.
         *
         * This function generates a sequence of random seed values using a random device.
         * It ensures enough seed values, which are used to initialize more complex
         * random number generators like Mersenne Twister.
         *
         * @return A seed sequence containing the generated seed values.
         */
        template <typename Engine>
        std::seed_seq MakeSeedSeq()
        {
            std::random_device rd;
            constexpr std::size_t n = SeedWordCount<Engine>();
            std::vector<std::seed_seq::result_type> seedData;
            seedData.reserve(n);
            std::generate_n(std::back_inserter(seedData), n, std::ref(rd));

            return {std::begin(seedData), std::end(seedData)};
        }

        /**
         * @brief Applies a series of bitwise and multiplication operations to mix the bits of a 64-bit unsigned integer.
         *
         * This function performs a bitwise manipulation technique commonly used in hash algorithms
         * to mix bits of the input number, ensuring a more uniform distribution of hash values.
         *
         * @param x The 64-bit unsigned integer to be mixed.
         * @return The mixed 64-bit unsigned integer.
         */
        inline std::uint64_t Mix64(std::uint64_t x)
        {
            x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
            x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
            x ^= (x >> 31);
            return x;
        }
    } // namespace detail

    /**
     * @brief Creates and returns a seeded random number generator (RNG) of the specified Engine type.
     *
     * @tparam Engine The type of the RNG engine to create. Must satisfy the requirements of
     * std::uniform_random_bit_generator and be either constructible from or seedable with a std::seed_seq.
     *
     * @return A seeded RNG of the specified Engine type. If the Engine can be directly constructed
     * using a std::seed_seq, the generator is initialized in this manner. Otherwise, the Engine is
     * default-constructed and seeded using its seed member function. The seeding process is based on
     * random seed material derived from std::random_device.
     *
     * @note This function is marked with [[nodiscard]] to prevent the created RNG from being
     * accidentally discarded, ensuring robust random number generation. The randomness quality
     * depends on the underlying implementation of std::random_device.
     */
    template <typename Engine>
        requires (std::uniform_random_bit_generator<Engine> &&
            detail::SeedSeqConstructibleOrSeedable<Engine>)
    [[nodiscard]] Engine MakeSeededRng()
    {
        if constexpr (auto seq = detail::MakeSeedSeq<Engine>();
            requires { Engine{seq}; })
        {
            return Engine(seq);
        }
        else
        {
            Engine eng{};
            eng.seed(seq);
            return eng;
        }
    }

    /**
     * @brief Creates and returns a seeded random number generator (RNG) of the specified Engine type.
     *
     * @tparam Engine The type of the RNG engine to create. Must satisfy the requirements of
     * std::uniform_random_bit_generator and be either constructible from or seedable with a std::seed_seq.
     *
     * @param seedSize The number of 32-bit words to use as seed material. The random seed material
     * is generated using std::random_device.
     *
     * @return A seeded RNG of the specified Engine type. The generator is seeded using a seed sequence
     * constructed from the specified number of 32-bit seed words. If the Engine supports construction
     * from a std::seed_seq, it is constructed directly. Otherwise, it is default-constructed and seeded
     * via the seed member function.
     *
     * @note This function is marked with [[nodiscard]] to ensure the created RNG is not inadvertently
     * discarded. The quality and randomness of the RNG depend on the underlying std::random_device
     * implementation.
     */
    template <typename Engine>
        requires (std::uniform_random_bit_generator<Engine> &&
            detail::SeedSeqConstructibleOrSeedable<Engine>)
    [[nodiscard]] Engine MakeSeededRngWithWords(const std::size_t seedSize)
    {
        std::random_device rd;
        std::vector<std::seed_seq::result_type> seedData;
        seedData.reserve(seedSize);
        std::generate_n(std::back_inserter(seedData), seedSize, std::ref(rd));

        if constexpr (std::seed_seq seq(std::begin(seedData), std::end(seedData));
            requires { Engine{seq}; })
        {
            return Engine(seq);
        }
        else
        {
            Engine eng{};
            eng.seed(seq);
            return eng;
        }
    }

    /**
     * @brief Creates a custom-seeded random number generator engine with hashed seed processing.
     *
     * This function generates a random number generator engine of type `Engine` using a user-provided
     * seed and an optional hasher. The provided seed is hashed into a 64-bit state, which is processed
     * to produce a sequence of 32-bit integers for initializing the random number generator engine.
     *
     * @param seed The seed to initialize the random number generator, which will be hashed.
     * @param hasher An optional custom hasher to transform the seed. Defaults to the default-constructed `THasher`.
     * @return A random number generator engine of type `Engine` initialized using the hashed seed.
     */
    template <typename Engine, typename Seed, typename THasher = std::hash<Seed>>
        requires (std::uniform_random_bit_generator<Engine> &&
            psyengine::utils::detail::SeedSeqConstructibleOrSeedable<Engine> &&
            std::invocable<THasher, const Seed&>)
    [[nodiscard]] Engine MakeCustomSeededRngHashed(const Seed& seed, THasher hasher = {})
    {
        // Hash user-provided seed into a 64-bit state
        auto state = static_cast<std::uint64_t>(hasher(seed));
        // Strengthen the entropy distribution a bit
        state = detail::Mix64(state);

        // Expand into enough 32-bit words for Engine using splitmix64 progression
        constexpr std::size_t n = psyengine::utils::detail::SeedWordCount<Engine>();
        std::vector<std::seed_seq::result_type> seedData;
        seedData.reserve(n);

        for (std::size_t i = 0; i < n; ++i)
        {
            // advance state (golden ratio increment)
            state += 0x9E3779B97F4A7C15ULL;
            const std::uint64_t z = psyengine::utils::detail::Mix64(state);
            seedData.push_back(static_cast<std::uint32_t>(z)); // take 32-bit chunks
        }

        std::seed_seq seq(std::begin(seedData), std::end(seedData));
        if constexpr (requires { Engine{seq}; })
        {
            return Engine(seq);
        }
        else
        {
            Engine eng{};
            eng.seed(seq);
            return eng;
        }
    }

    /**
     * @brief Creates a custom-seeded random number generator (RNG) engine using a range of elements.
     *
     * This method generates a seed by hashing and combining values from the given range,
     * then constructs a random number generator using the resulting hashed seed.
     *
     * @tparam Engine The random number generator engine type to be created.
     * @tparam Range The type of the input range containing elements.
     * @tparam THasher The type of the hasher to be applied to each element in the range.
     * @param items The input range of elements to be hashed and combined.
     * @param elemHasher The hasher function or object to calculate a hash value for each element in the range.
     * @return Returns a custom-seeded random number generator engine initialized with the combined seed.
     */
    template <typename Engine, typename Range, typename THasher>
        requires (std::uniform_random_bit_generator<Engine> &&
            psyengine::utils::detail::SeedSeqConstructibleOrSeedable<Engine> &&
            requires(const Range& r) { std::begin(r); std::end(r); } &&
            std::invocable<THasher, const std::ranges::range_value_t<Range>&>)
    [[nodiscard]] Engine MakeCustomSeededRngHashedRange(const Range& items, THasher elemHasher)
    {
        std::uint64_t combined = 0xcbf29ce484222325ULL; // FNV-1a offset basis as a simple start
        for (const auto& v : items)
        {
            const auto h = static_cast<std::uint64_t>(elemHasher(v));
            // simple 64-bit combine
            combined ^= h + 0x9E3779B97F4A7C15ULL + (combined << 6) + (combined >> 2);
        }

        // Reuse the single-seed version with a lambda hasher
        struct PassThroughHash
        {
            std::uint64_t h;

            std::uint64_t operator()(std::uint64_t) const noexcept
            {
                return h;
            }
        };

        return MakeCustomSeededRngHashed<Engine, std::uint64_t>(combined, PassThroughHash{combined});
    }

    /**
     * @brief Creates and returns a seeded Mersenne Twister random number generator with a 32-bit state size.
     *
     * This function uses the MakeSeededRng function to initialize a Mersenne Twister engine
     * (std::mt19937) with a random seed value. The engine is seeded using random material generated
     * from a high-quality source, such as std::random_device.
     *
     * @return A seeded Mersenne Twister random number generator (std::mt19937). The generator is
     * initialized to provide robust and high-quality random number generation.
     *
     * @note Proper seeding ensures that the RNG produces a random enough and non-repeating
     * sequence of numbers.
     */
    inline auto MakeMersenne32()
    {
        return MakeSeededRng<std::mt19937>();
    }

    /**
     * @brief Creates and returns a seeded Mersenne Twister random number generator with a 64-bit state size.
     *
     * This function uses the MakeSeededRng function to initialize a Mersenne Twister engine
     * (std::mt19937_64) with a random seed value. The engine is seeded using random material generated
     * from a high-quality source, such as std::random_device.
     *
     * @return A seeded Mersenne Twister random number generator (std::mt19937_64). The generator is
     * initialized to provide robust and high-quality random number generation.
     *
     * @note Proper seeding ensures that the RNG produces a random enough and non-repeating
     * sequence of numbers.
     */
    inline auto MakeMersenne64()
    {
        return MakeSeededRng<std::mt19937_64>();
    }

    /**
     * @brief Creates a 32-bit Mersenne Twister random number generator with a custom seeded hash.
     *
     * This function generates a Mersenne64 random number generator seeded using the given seed
     * and an optional custom hash function.
     *
     * @param seed The seed value used for initializing the random number generator.
     * @param hasher The custom hash function used for seeding. Defaults to an empty instance of the specified type.
     * @return A Mersenne64 random number generator initialized with the provided seed and hash function.
     */
    template <typename Seed, typename THasher = std::hash<Seed>>
    auto MakeMersenne32CustomSeededHash(const Seed& seed, THasher hasher = {})
    {
        return MakeCustomSeededRngHashed<std::mt19937, Seed, THasher>(seed, hasher);
    }

    /**
     * @brief Creates a 64-bit Mersenne Twister random number generator with a custom seeded hash.
     *
     * This function generates a Mersenne64 random number generator seeded using the given seed
     * and an optional custom hash function.
     *
     * @param seed The seed value used for initializing the random number generator.
     * @param hasher The custom hash function used for seeding. Defaults to an empty instance of the specified type.
     * @return A Mersenne64 random number generator initialized with the provided seed and hash function.
     */
    template <typename Seed, typename THasher = std::hash<Seed>>
    auto MakeMersenne64CustomSeededHash(const Seed& seed, THasher hasher = {})
    {
        return MakeCustomSeededRngHashed<std::mt19937_64, Seed, THasher>(seed, hasher);
    }

    template <std::floating_point T = float>
    [[nodiscard]] T RandomFloat(auto& rng, T min = T{0}, T max = T{1})
    {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(rng);
    }

    template <std::integral T = int>
    [[nodiscard]] T RandomInt(auto& rng, T min, T max)
    {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(rng);
    }

    [[nodiscard]] bool RandomBool(auto& rng, const double probability = 0.5)
    {
        std::bernoulli_distribution const dist(probability);
        return dist(rng);
    }

    // Random element selection
    template <typename Container>
    [[nodiscard]] auto& RandomElement(auto& rng, Container& container)
    {
        auto dist = std::uniform_int_distribution<std::size_t>(0, container.size() - 1);
        auto it = std::begin(container);
        std::advance(it, dist(rng));
        return *it;
    }

    // Shuffle wrapper
    template <typename Container>
    void Shuffle(auto& rng, Container& container)
    {
        std::shuffle(std::begin(container), std::end(container), rng);
    }

    // Thread-safe global RNG
    class GlobalRng
    {
    public:
        static auto& get()
        {
            if (!initialized_)
            {
                rng_ = MakeMersenne64();
                initialized_ = true;
            }
            return rng_;
        }

        static void seed(auto&& seed)
        {
            rng_ = MakeMersenne64CustomSeededHash(seed);
            initialized_ = true;
        }

    private:
        static thread_local std::mt19937_64 rng_;
        static thread_local bool initialized_;
    };

    // Convenience functions using global RNG
    template <std::floating_point T = float>
    [[nodiscard]] T Random(T min = T{0}, T max = T{1})
    {
        return RandomFloat(GlobalRng::get(), min, max);
    }

    template <std::integral T = int>
    [[nodiscard]] T RandomInt(T min, T max)
    {
        return RandomInt(GlobalRng::get(), min, max);
    }
}

#endif //PSYGINE_RANDOM_HPP
