#include <vector>
#include <random>
#include <gtest/gtest.h>

class Solution {
public:
    explicit Solution(std::vector<int> nums,
                      std::mt19937::result_type seed = std::mt19937::default_seed) :
        nums_(std::move(nums)),
        rng_(seed) {}

    [[nodiscard]] const std::vector<int> &reset() const {
        return nums_;
    }

    [[nodiscard]] std::vector<int> shuffle() {
        auto shuffled = nums_;
        std::shuffle(shuffled.begin(), shuffled.end(), rng_);
        return shuffled;
    }

private:
    std::vector<int> nums_;
    std::mt19937 rng_;
};

TEST(Solution, shuffleAnArray) {
    constexpr std::size_t n_trials = 200'000;
    constexpr std::size_t percent_tolerance = 2;

    auto check = [](int len, std::size_t seed) {
        // fill nums with integers from 0 to len - 1 for easy validation
        auto nums = std::vector<int>(len, 0);
        std::iota(nums.begin(), nums.end(), 0);

        auto sol = Solution(nums, seed);
        EXPECT_EQ(sol.reset(), nums);

        // run shuffle a lot and record positions of the shuffled indices in
        // a 2D array of [index][location]
        auto distribution = std::vector<int>(len * len, 0);
        for (std::size_t t = 0; t < n_trials; ++t) {
            auto shuffled = sol.shuffle();
            // record the position of each element in the distribution array
            for (std::size_t i = 0; i < len; ++i) {
                distribution[i * len + shuffled[i]]++;
            }
            // check that the sorted array is the same as the initial one
            std::sort(shuffled.begin(), shuffled.end());
            EXPECT_EQ(shuffled, nums);
        }

        // check that reset still works
        EXPECT_EQ(sol.reset(), nums);

        // check that the distribution counts are within tolerance
        auto expected_count = static_cast<std::int64_t>(n_trials / len);
        auto tolerance = static_cast<std::int64_t>(percent_tolerance * (expected_count / 100));
        for (auto count : distribution) {
            EXPECT_LT(std::abs(count - expected_count), tolerance);
        }
    };

    check(8, std::mt19937::default_seed);
    check(12, std::mt19937::default_seed);
}

