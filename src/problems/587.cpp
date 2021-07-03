#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

// https://leetcode.com/explore/interview/card/top-interview-questions-easy/96/sorting-and-searching/587/
class Solution {
public:
    static void merge(std::vector<int>& nums1, int m,
                      std::vector<int>& nums2, int n) {
        assert(nums1.size() == m + n);
        assert(nums2.size() == n);

        // shift nums1 so that the `n` zeros are at the beginning of the range
        std::rotate(nums1.rbegin(), nums1.rbegin() + n, nums1.rend());

        auto out = nums1.begin();
        auto it1 = nums1.begin() + n;
        auto it2 = nums2.begin();

        while (true) {
            if (it1 == nums1.end()) {
                std::copy(it2, nums2.end(), out);
                return;
            } else if (it2 == nums2.end()) {
                std::copy(it1, nums1.end(), out);
                return;
            }

            if (*it1 < *it2) {
                *out++ = *it1++;
            } else {
                *out++ = *it2++;
            }
        }
    }

    static void test(std::vector<int> nums1, int m,
                     std::vector<int> nums2, int n,
                     const std::vector<int> &expected) {
        merge(nums1, m, nums2, n);
        EXPECT_EQ(nums1, expected);
    }
};

TEST(Solution, mergeSortedArray) {
    Solution::test(
            {1, 2, 3, 0, 0, 0}, 3,
            {2, 5, 6}, 3,
            {1, 2, 2, 3, 5, 6}
    );
    Solution::test(
            {1, 2, 3, 0, 0, 0}, 3,
            {1, 2, 2}, 3,
            {1, 1, 2, 2, 2, 3}
    );
    // m == 0
    Solution::test(
            {0, 0, 0}, 0,
            {1, 2, 3}, 3,
            {1, 2, 3}
    );
    // n == 0
    Solution::test(
            {1, 2, 3}, 3,
            {}, 0,
            {1, 2, 3}
    );
    // m == n == 0
    Solution::test(
            {}, 0,
            {}, 0,
            {}
    );
    // nums1 < nums2
    Solution::test(
            {1, 2, 3, 0, 0, 0}, 3,
            {4, 5, 6}, 3,
            {1, 2, 3, 4, 5, 6}
    );
    // nums2 < nums1
    Solution::test(
            {4, 5, 6, 0, 0, 0}, 3,
            {1, 2, 3}, 3,
            {1, 2, 3, 4, 5, 6}
    );
    Solution::test(
            {3, 0, 0, 0}, 1,
            {1, 2, 4}, 3,
            {1, 2, 3, 4}
    );
    Solution::test(
            {1, 2, 4, 0}, 3,
            {3}, 1,
            {1, 2, 3, 4}
    );
}
