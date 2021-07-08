#include <vector>
#include <optional>

class Iterator {
public:
	explicit Iterator(const std::vector<int>& nums) : current_(nums.begin()), end_(nums.end()) {}
	Iterator(const Iterator& iter) = default;

	// Returns the next element in the iteration.
	int next() {
	    return *current_++;
	}

    // Returns true if the iteration has more elements.
	[[nodiscard]] bool hasNext() const {
	    return current_ != end_;
	}

private:
    std::vector<int>::const_iterator current_;
    std::vector<int>::const_iterator end_;
};

class PeekingIterator : public Iterator {
public:
    explicit PeekingIterator(const std::vector<int>& nums) : Iterator(nums) {
        advance();
    }

    // Returns the next element in the iteration without advancing the iterator.
    int peek() {
        return *next_;
    }

    int next() {
        auto value = *next_;
        advance();
        return value;
    }

    [[nodiscard]] bool hasNext() const {
        return next_.has_value();
    }

private:
    void advance() {
        if (Iterator::hasNext()) {
            next_ = Iterator::next();
        } else {
            next_ = std::nullopt;
        }
    }

    std::optional<int> next_;
};

#include <gtest/gtest.h>

TEST(Solution, PeekingIterator) {
    std::vector<int> one_item = {1};
    auto item_iter = PeekingIterator(one_item);
    EXPECT_EQ(item_iter.hasNext(), true);
    EXPECT_EQ(item_iter.peek(), 1);
    EXPECT_EQ(item_iter.next(), 1);
    EXPECT_EQ(item_iter.hasNext(), false);

    std::vector<int> items = {1, 2, 3, 4};
    auto iter = PeekingIterator(items);

    EXPECT_EQ(iter.peek(), 1);
    EXPECT_EQ(iter.next(), 1);
    EXPECT_EQ(iter.peek(), 2);
    EXPECT_EQ(iter.next(), 2);
    EXPECT_EQ(iter.peek(), 3);
    EXPECT_EQ(iter.peek(), 3);
    EXPECT_EQ(iter.next(), 3);
    EXPECT_EQ(iter.hasNext(), true);
    EXPECT_EQ(iter.next(), 4);
    EXPECT_EQ(iter.hasNext(), false);

    std::vector<int> no_items = {};
    auto empty_iter = PeekingIterator(no_items);
    EXPECT_EQ(empty_iter.hasNext(), false);
}
