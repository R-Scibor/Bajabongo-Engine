#pragma once

#include <cstdint>
#include <random>

namespace engine {

    /**
     * @brief Shared mt19937 generator, seeded once (random_device or an explicit seed).
     *
     * Integer helpers use inclusive bounds, matching `rand() % n` as the half-open
     * range [0, n). Real helpers use [lo, hi), matching std::uniform_real_distribution.
     * If lo >= hi, real() returns lo so a zero-width spread stays at zero.
     */
    class Random {
    public:
        Random() : Random(static_cast<std::uint32_t>(std::random_device{}())) {}

        explicit Random(std::uint32_t seed) { this->seed(seed); }

        void seed(std::uint32_t seed) {
            m_seed = seed;
            m_engine.seed(seed);
        }

        std::uint32_t currentSeed() const { return m_seed; }

        /// Inclusive integer range [lo, hi]. If lo > hi, returns lo.
        int integer(int lo, int hi) {
            if (lo > hi) return lo;
            return std::uniform_int_distribution<int>(lo, hi)(m_engine);
        }

        /// Integer in [0, n). Matches the range of `rand() % n` for n > 0.
        /// If n <= 1, returns 0.
        int below(int n) {
            if (n <= 1) return 0;
            return integer(0, n - 1);
        }

        /// Real in [lo, hi). If lo >= hi, returns lo.
        float real(float lo, float hi) {
            if (!(lo < hi)) return lo;
            return std::uniform_real_distribution<float>(lo, hi)(m_engine);
        }

    private:
        std::mt19937 m_engine;
        std::uint32_t m_seed = 0;
    };

} // namespace engine
