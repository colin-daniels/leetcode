#include <unordered_map>
#include <gtest/gtest.h>

class LRUCache {
public:
    explicit LRUCache(std::size_t capacity) :
            capacity_(capacity),
            cache_(),
            oldest_(nullptr),
            newest_(nullptr)
    {
        cache_.reserve(capacity_);
    }

    /// @return The value of key if it exists, otherwise -1.
    int get(int key) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            update(it->second);
            return it->second.value_;
        } else {
            return -1;
        }
    }

    void put(int key, int value) {
        // early exit for empty case
        if (capacity_ == 0) { return; }

        // first just search for the key, note that we can't really
        // use unordered_map::insert_or_assign here because we need
        // the old entry
        auto it = cache_.find(key);

        if (it != cache_.end()) {
            update(it->second, value);
        } else {
            // check if we're going to go over capacity
            if (cache_.size() + 1 > capacity_) {
                evict_oldest();
            }

            // add it
            insert_newest(key, value);
        }
    }

private:
    struct Entry {
        Entry *prev_;
        Entry *next_;
        const int key_;
        int value_;

        void unlink() const {
            if (prev_) { prev_->next_ = next_; }
            if (next_) { next_->prev_ = prev_; }
        }

        void link() {
            if (prev_) { prev_->next_ = this; }
            if (next_) { next_->prev_ = this; }
        }
    };

    void update(Entry &entry, std::optional<int> value = std::nullopt) {
        // early exit if this is already the newest entry,
        // this also takes care of the single-entry case
        if (&entry == newest_) {
            if (value) { entry.value_ = *value; }
            return;
        }

        // update the oldest entry if needed
        if (&entry == oldest_) { oldest_ = entry.next_; }
        // re-direct old links
        entry.unlink();
        // set new links
        entry.prev_ = newest_;
        entry.next_ = nullptr;
        if (value) { entry.value_ = *value; }
        entry.link();

        // update newest
        newest_ = &entry;
    }

    void insert_newest(int key, int value) {
        auto [it, inserted] = cache_.try_emplace(key, Entry{nullptr, nullptr, key, value});
        assert(inserted);
        if (capacity_ != 1) { it->second.prev_ = newest_; }
        newest_ = &it->second;
        newest_->link();
        // if this is the first entry added or capacity is 1
        if (!oldest_) { oldest_ = newest_; }
    }

    void evict_oldest() {
        assert(oldest_);
        auto new_oldest = oldest_->next_;
        // unlink
        oldest_->unlink();
        cache_.erase(oldest_->key_);
        // update oldest
        oldest_ = new_oldest;
    }

    const std::size_t capacity_;
    std::unordered_map<int, Entry> cache_;
    Entry* newest_ = nullptr;
    Entry* oldest_ = nullptr;
};

TEST(Solution, LRUCache) {
    // capacity = 4
    {
        LRUCache cache(4);
        cache.put(1, 1);
        cache.put(2, 2);
        cache.put(3, 3);
        cache.put(4, 4);

        EXPECT_EQ(cache.get(2), 2);
        EXPECT_EQ(cache.get(3), 3);
        cache.put(5, 5);
        EXPECT_EQ(cache.get(1), -1);
        cache.put(6, 6);
        EXPECT_EQ(cache.get(4), -1);
        EXPECT_EQ(cache.get(2), 2);
        cache.put(7, 7);
        EXPECT_EQ(cache.get(3), -1);
        cache.put(7, 7);
        cache.put(7, 5);
        EXPECT_EQ(cache.get(7), 5);
        cache.put(7, 7);
        EXPECT_EQ(cache.get(7), 7);
        EXPECT_EQ(cache.get(6), 6);
        EXPECT_EQ(cache.get(5), 5);
        EXPECT_EQ(cache.get(2), 2);
    }

    // capacity = 2
    {
        LRUCache cache(2);
        cache.put(1, 1);
        cache.put(2, 2);
        EXPECT_EQ(cache.get(1), 1);
        cache.put(3, 3);
        EXPECT_EQ(cache.get(2), -1);
        cache.put(4, 4);
        EXPECT_EQ(cache.get(1), -1);
        EXPECT_EQ(cache.get(3), 3);
        EXPECT_EQ(cache.get(4), 4);
        // repeat oldest & newest
        EXPECT_EQ(cache.get(3), 3);
        EXPECT_EQ(cache.get(3), 3);
        cache.put(4, 44);
        cache.put(2, 22);
        EXPECT_EQ(cache.get(3), -1);
        EXPECT_EQ(cache.get(4), 44);
    }
    // capacity = 1
    {
        LRUCache cache(1);
        cache.put(2, 1);
        EXPECT_EQ(cache.get(2), 1);
        cache.put(3, 2);
        EXPECT_EQ(cache.get(2), -1);
        EXPECT_EQ(cache.get(3), 2);
    }
    // capacity = 1
    {
        LRUCache cache(1);
        EXPECT_EQ(cache.get(1), -1);
        cache.put(1, 1); // cache is {1=1}
        EXPECT_EQ(cache.get(1), 1);
        cache.put(2, 2); // cache is {2=2}
        EXPECT_EQ(cache.get(1), -1);
        EXPECT_EQ(cache.get(2), 2);
        cache.put(2, 3); // cache is {2=3}
        EXPECT_EQ(cache.get(2), 3);
    }
    // capacity = 0
    {
        LRUCache cache(0);
        EXPECT_EQ(cache.get(1), -1);
        cache.put(1, 1);
        EXPECT_EQ(cache.get(1), -1);
    }
}
