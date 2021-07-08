#include <memory>
#include <stack>

// Definition for a binary tree node.
struct TreeNode {
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
    int val;

    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(TreeNode &&left, int x, TreeNode &&right) :
            val(x),
            left(std::make_unique<TreeNode>(std::move(left))),
            right(std::make_unique<TreeNode>(std::move(right))) {}

    TreeNode(int x, TreeNode &&right) :
            val(x),
            left(),
            right(std::make_unique<TreeNode>(std::move(right))) {}

    TreeNode(TreeNode &&left, int x) :
            val(x),
            left(std::make_unique<TreeNode>(std::move(left))),
            right() {}
};

class BSTIterator {
public:
    explicit BSTIterator(TreeNode* root) {
        stack_.emplace(*root);
        nextLeaf();
    }

    int next() {
        auto &top = stack_.top();
        auto value = top.node_->val;
        top.visited_ = true;

        if (top.node_->right) {
            stack_.emplace(*top.node_->right);
            nextLeaf();
        } else {
            stack_.pop();
            nextNode();
        }

        return value;
    }

    [[nodiscard]] bool hasNext() const {
        return !stack_.empty();
    }

private:
    void nextLeaf() {
        do {
            auto &top = *stack_.top().node_;
            if (top.left) {
                stack_.emplace(*top.left);
            } else {
                // continue until we get to a node with no left child
                break;
            }
        } while (true);
    }

    void nextNode() {
        while (!stack_.empty() && stack_.top().visited_) {
            stack_.pop();
        }
    }

    struct NodeIter {
        explicit NodeIter(TreeNode &node) : node_(&node) {}

        TreeNode *node_;
        bool visited_ = false;
    };

    std::stack<NodeIter> stack_;
};

#include <gtest/gtest.h>

TEST(Solution, BSTIterator) {
    using TN = TreeNode;
    auto check = [](TreeNode node, int num_expected) {
        std::vector<int> actual;
        std::vector<int> expected;

        auto iter = BSTIterator(&node);
        for (int i = 1; i <= num_expected; ++i) {
            expected.push_back(i);
            if (iter.hasNext()) {
                actual.push_back(iter.next());
            }
        }

        EXPECT_EQ(actual, expected);
        EXPECT_FALSE(iter.hasNext());
    };

    check(TN(1), 1);
    check(TN(TN(1), 2, TN(3)), 3);
    check(TN(TN(TN(1), 2), 3), 3);
    check(TN(1, TN(2, TN(3))), 3);
    check(TN(TN(1, TN(2)), 3, TN(TN(4), 5)), 5);
    check(TN(TN(TN(1), 2, TN(3)), 4, TN(TN(5), 6, TN(7))), 7);
}
