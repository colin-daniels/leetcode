#include <gtest/gtest.h>

class Solution {
public:
    template<class F>
    static int firstBadVersion(int n, F &&isBadVersion) {
        int low = 1;
        int high = n;

        while (low < high) {
            int mid = (high - low) / 2 + low;
            if (isBadVersion(mid)) {
                // first bad version is in [low, mid]
                high = mid;
            } else {
                // first bad version is in (mid, high]
                low = mid + 1;
            }
        }

        return high;
    }

    static void test(int n, int bad) {
        auto isBad = [bad](int m) { return m >= bad; };
        auto actual = firstBadVersion(n, isBad);
        EXPECT_EQ(actual, bad);
    }
};

TEST(Solution, firstBadVersion) {
    auto testAllN = [](int n) {
        for (int i = 1; i < n; ++i)
            Solution::test(n, i);
    };

    testAllN(1);
    testAllN(5);
    testAllN(6);
    testAllN(12);
}
