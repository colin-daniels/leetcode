#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

// https://leetcode.com/explore/interview/card/top-interview-questions-easy/92/array/727/
class Solution {
public:
    static std::size_t removeDuplicates(std::vector<int>& nums) {
        auto end = std::unique(nums.begin(), nums.end());
        return std::distance(nums.begin(), end);
    }

    static void test(std::vector<int> nums, const std::vector<int> &expected) {
        std::size_t new_len = removeDuplicates(nums);
        nums.resize(std::min(new_len, nums.size()));

        EXPECT_EQ(new_len, expected.size());
        EXPECT_EQ(nums, expected);
    }
};

TEST(Solution, removeDuplicatesFromSortedArray) {
    Solution::test({}, {});
    Solution::test({1}, {1});
    Solution::test({1, 1}, {1});
    Solution::test({1, 1, 1}, {1});
    Solution::test({1, 1, 1, 2, 2, 3}, {1, 2, 3});
    Solution::test({0, 0, 1, 1, 1, 1, 2, 3, 3}, {0, 1, 2, 3});
}
