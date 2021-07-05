#include <vector>
#include <stack>
#include <variant>

class NestedInteger {
public:
    NestedInteger(int value) : data_(value) {}
    NestedInteger(std::vector<NestedInteger> list) : data_(std::move(list)) {}

    // Return true if this NestedInteger holds a single integer, rather than a nested list.
    [[nodiscard]] bool isInteger() const {
        return std::holds_alternative<int>(data_);
    }

    // Return the single integer that this NestedInteger holds, if it holds a single integer
    // The result is undefined if this NestedInteger holds a nested list
    [[nodiscard]] int getInteger() const {
        return std::get<int>(data_);
    }

    // Return the nested list that this NestedInteger holds, if it holds a nested list
    // The result is undefined if this NestedInteger holds a single integer
    [[nodiscard]] const std::vector<NestedInteger> &getList() const {
        return std::get<std::vector<NestedInteger>>(data_);
    }

private:
    std::variant<int, std::vector<NestedInteger>> data_;
};

class NestedIterator {
public:
    explicit NestedIterator(const std::vector<NestedInteger> &nestedList) : stack_{} {
        stack_.push(std::pair{nestedList.begin(), nestedList.end()});
        // advance until we get to a single int
        advance();
    }

    int next() {
        auto &[current, end] = stack_.top();
        auto value = current->getInteger();
        // go to next
        ++current;
        advance();

        return value;
    }

    [[nodiscard]] bool hasNext() const {
        return !stack_.empty();
    }
private:
    using item_iter = std::vector<NestedInteger>::const_iterator;
    std::stack<std::pair<item_iter, item_iter>> stack_;

    void advance() {
        while (!stack_.empty()) {
            auto &[current, end] = stack_.top();
            if (current == end) {
                // no items left on this level, go up one
                stack_.pop();
            } else if (!current->isInteger()) {
                // this item isn't a single integer, add
                // the begin/end iterators to the stack
                // and advance `current` to indicate we're
                // iterating over the current item
                auto &list = current->getList();
                ++current;
                stack_.emplace(list.begin(), list.end());
            } else {
                break; // current->isInteger() == true
            }
        }
    }
};

#include <gtest/gtest.h>

TEST(Solution, FlattenNestedIterator) {
    auto checkCase = [](const std::vector<NestedInteger>& nested,
                        const std::vector<int>& expected) {

        NestedIterator it{nested};
        std::vector<int> actual;
        while (it.hasNext()) {
            actual.push_back(it.next());
        }

        EXPECT_EQ(actual, expected);
    };

    checkCase({
                      std::vector<NestedInteger>{1, std::vector<NestedInteger>{2, 3}},
                      4,
                      std::vector<NestedInteger>{std::vector<NestedInteger>{}},
                      std::vector<NestedInteger>{5, 6},
                      7,
                      std::vector<NestedInteger>{}
              }, {1, 2, 3, 4, 5, 6, 7});

    checkCase({1}, {1});
    checkCase({1, 2}, {1, 2});
    checkCase({std::vector<NestedInteger>{}, 1}, {1});
    checkCase({std::vector<NestedInteger>{std::vector<NestedInteger>{std::vector<NestedInteger>{1}}}}, {1});
    checkCase({}, {});
    checkCase({std::vector<NestedInteger>{}}, {});
}
