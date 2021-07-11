use crate::util::{CircBuf, KWayMergeIterator};
use std::collections::HashSet;

#[derive(Copy, Clone, Default, PartialOrd, PartialEq, Ord, Eq)]
struct Tweet {
    pub time: u32,
    pub id: i32,
}

pub struct Twitter {
    time: u32,
    users: Vec<TweetUser>,
}

impl Twitter {
    const NEWSFEED_SIZE: usize = 10;
    const MAX_USERID: i32 = 500;
}

#[derive(Clone, Default)]
struct TweetUser {
    tweets: CircBuf<Tweet, { Twitter::NEWSFEED_SIZE }>,
    following: HashSet<i32>,
}

impl TweetUser {
    #[inline]
    fn post_tweet(&mut self, tweet: Tweet) {
        self.tweets.push(tweet);
    }

    #[inline]
    fn tweets(&self) -> impl Iterator<Item = &Tweet> {
        self.tweets.iter()
    }

    #[inline]
    fn follow(&mut self, user_id: i32) {
        self.following.insert(user_id);
    }

    #[inline]
    fn unfollow(&mut self, user_id: i32) {
        self.following.remove(&user_id);
    }
}

impl Twitter {
    /** Initialize your data structure here. */
    pub fn new() -> Self {
        Self {
            users: vec![Default::default(); Self::MAX_USERID as usize],
            time: 0,
        }
    }

    #[inline]
    fn valid_user(user_id: i32) -> bool {
        1 <= user_id && user_id <= Self::MAX_USERID
    }

    #[inline]
    fn user(&self, user_id: i32) -> Option<&TweetUser> {
        if Self::valid_user(user_id) {
            Some(&self.users[(user_id - 1) as usize])
        } else {
            None
        }
    }

    #[inline]
    fn user_mut(&mut self, user_id: i32) -> Option<&mut TweetUser> {
        if Self::valid_user(user_id) {
            Some(&mut self.users[(user_id - 1) as usize])
        } else {
            None
        }
    }

    /** Compose a new tweet. */
    pub fn post_tweet(&mut self, user_id: i32, tweet_id: i32) {
        let mut time = self.time;
        if let Some(user) = self.user_mut(user_id) {
            user.post_tweet(Tweet { id: tweet_id, time });
            time = time + 1;
        }
        self.time = time;
    }
    pub fn post_tweets(&mut self, user_id: i32, tweets: impl IntoIterator<Item = i32>) {
        for tweet in tweets {
            self.post_tweet(user_id, tweet);
        }
    }

    /** Retrieve the 10 most recent tweet ids in the user's news feed. Each item in the news
    feed must be posted by users who the user followed or by the user herself. Tweets
    must be ordered from most recent to least recent. */
    pub fn get_news_feed(&self, user_id: i32) -> Vec<i32> {
        if let Some(user) = self.user(user_id) {
            let tweet_iterators = user
                .following
                .iter()
                .filter_map(|&followee_id| self.user(followee_id))
                .chain(std::iter::once(user))
                .map(TweetUser::tweets);

            let merger = KWayMergeIterator::new(tweet_iterators);

            // get the NEWSFEED_SIZE users with the most recent tweets
            merger
                .take(Self::NEWSFEED_SIZE)
                .map(|tweet| tweet.id)
                .collect()
        } else {
            vec![]
        }
    }

    /** Follower follows a followee. If the operation is invalid, it should be a no-op. */
    pub fn follow(&mut self, follower_id: i32, followee_id: i32) {
        if let Some(user) = self.user_mut(follower_id) {
            if Self::valid_user(followee_id) {
                user.follow(followee_id);
            }
        }
    }

    /** Follower unfollows a followee. If the operation is invalid, it should be a no-op. */
    pub fn unfollow(&mut self, follower_id: i32, followee_id: i32) {
        if let Some(user) = self.user_mut(follower_id) {
            user.unfollow(followee_id);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Twitter;
    use std::collections::{HashMap, HashSet};

    struct RefTweet {
        user_id: i32,
        tweet_id: i32,
    }

    #[derive(Default)]
    struct RefTwitter {
        tweets: Vec<RefTweet>,
        users: HashMap<i32, HashSet<i32>>,
        method_calls: Vec<String>,
        method_params: Vec<String>,
        unique_tweets: HashSet<i32>,
    }

    impl RefTwitter {
        pub fn new() -> Self {
            RefTwitter::default()
        }

        pub fn post_tweet(&mut self, user_id: i32, tweet_id: i32) {
            assert!(!self.unique_tweets.contains(&tweet_id));
            self.unique_tweets.insert(tweet_id);

            self.tweets.push(RefTweet { user_id, tweet_id });

            self.method_calls.push("\"postTweet\"".into());
            self.method_params
                .push(format!("[{},{}]", user_id, tweet_id));
        }

        pub fn get_news_feed(&mut self, user_id: i32) -> Vec<i32> {
            self.method_calls.push("\"getNewsFeed\"".into());
            self.method_params.push(format!("[{}]", user_id));

            if let Some(following) = self.users.get(&user_id) {
                self.tweets
                    .iter()
                    .rev()
                    .filter(|tweet| tweet.user_id == user_id || following.contains(&tweet.user_id))
                    .take(10)
                    .map(|tweet| tweet.tweet_id)
                    .collect()
            } else {
                self.tweets
                    .iter()
                    .rev()
                    .filter(|tweet| tweet.user_id == user_id)
                    .take(10)
                    .map(|tweet| tweet.tweet_id)
                    .collect()
            }
        }

        pub fn follow(&mut self, follower_id: i32, followee_id: i32) {
            self.method_calls.push("\"follow\"".into());
            self.method_params
                .push(format!("[{},{}]", follower_id, followee_id));

            self.users
                .entry(follower_id)
                .or_default()
                .insert(followee_id);
        }

        pub fn unfollow(&mut self, follower_id: i32, followee_id: i32) {
            self.method_calls.push("\"unfollow\"".into());
            self.method_params
                .push(format!("[{},{}]", follower_id, followee_id));

            use std::collections::hash_map::Entry::Occupied;
            if let Occupied(mut entry) = self.users.entry(follower_id) {
                entry.get_mut().remove(&followee_id);
            }
        }

        pub fn post_tweets(&mut self, user_id: i32, tweets: impl IntoIterator<Item = i32>) {
            for tweet in tweets {
                self.post_tweet(user_id, tweet);
            }
        }
    }

    #[test]
    fn example_test() {
        let twitter = &mut Twitter::new();
        twitter.post_tweet(1, 5);
        assert_eq!(twitter.get_news_feed(1), vec![5]);
        twitter.follow(1, 2);
        twitter.post_tweet(2, 6);
        assert_eq!(twitter.get_news_feed(1), vec![6, 5]);
        twitter.unfollow(1, 2);
        assert_eq!(twitter.get_news_feed(1), vec![5]);
    }

    #[test]
    fn example_test2() {
        let mut twitter = Twitter::new();
        twitter.post_tweet(1, 3);
        twitter.post_tweet(2, 4);
        twitter.post_tweets(1, [5, 7, 8, 9, 10, 11, 12, 13, 14]);
        assert_eq!(
            twitter.get_news_feed(1),
            vec![14, 13, 12, 11, 10, 9, 8, 7, 5, 3]
        );
        twitter.follow(1, 2);
        twitter.post_tweet(2, 6);
        assert_eq!(
            twitter.get_news_feed(1),
            vec![6, 14, 13, 12, 11, 10, 9, 8, 7, 5]
        );
        twitter.unfollow(1, 2);
        assert_eq!(
            twitter.get_news_feed(1),
            vec![14, 13, 12, 11, 10, 9, 8, 7, 5, 3]
        );
        assert_eq!(twitter.get_news_feed(2), vec![6, 4]);
        twitter.follow(1, 2);
        assert_eq!(
            twitter.get_news_feed(1),
            vec![6, 14, 13, 12, 11, 10, 9, 8, 7, 5]
        );
        twitter.post_tweet(1, 15);
        twitter.follow(2, 1);
        assert_eq!(
            twitter.get_news_feed(1),
            vec![15, 6, 14, 13, 12, 11, 10, 9, 8, 7]
        );
        assert_eq!(
            twitter.get_news_feed(2),
            vec![15, 6, 14, 13, 12, 11, 10, 9, 8, 7]
        );
    }

    #[test]
    fn empty_tests() {
        let twitter = &mut Twitter::new();

        assert_eq!(twitter.get_news_feed(1), vec![]);
        assert_eq!(twitter.get_news_feed(Twitter::MAX_USERID), vec![]);

        assert_eq!(twitter.get_news_feed(0), vec![]);
        assert_eq!(twitter.get_news_feed(Twitter::MAX_USERID + 1), vec![]);

        // post_tweet should be a no-op for an out-of-range user_id
        twitter.post_tweet(0, 123);
        twitter.post_tweet(Twitter::MAX_USERID + 1, 456);
        assert_eq!(twitter.get_news_feed(0), vec![]);
        assert_eq!(twitter.get_news_feed(Twitter::MAX_USERID + 1), vec![]);
    }

    #[test]
    fn interleaved() {
        let twitter = &mut Twitter::new();
        let ref_twitter = &mut RefTwitter::new();

        for (user_id, tweet_id) in [
            (1, 100),
            (2, 200),
            (3, 300),
            (3, 301),
            (3, 302),
            (1, 101),
            (2, 201),
            (1, 102),
            (2, 202),
            (2, 203),
            (2, 204),
            (2, 205),
            (2, 206),
            (2, 207),
            (2, 208),
            (2, 209),
            (2, 210),
            (1, 103),
            (1, 104),
        ] {
            twitter.post_tweet(user_id, tweet_id);
            ref_twitter.post_tweet(user_id, tweet_id);
            assert_eq!(
                twitter.get_news_feed(user_id),
                ref_twitter.get_news_feed(user_id)
            );
        }
        twitter.post_tweets(4, 400..=420);
        ref_twitter.post_tweets(4, 400..=420);

        assert_eq!(twitter.get_news_feed(1), ref_twitter.get_news_feed(1));
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));
        assert_eq!(twitter.get_news_feed(3), ref_twitter.get_news_feed(3));
        assert_eq!(twitter.get_news_feed(4), ref_twitter.get_news_feed(4));

        twitter.follow(1, 3);
        ref_twitter.follow(1, 3);
        assert_eq!(twitter.get_news_feed(1), ref_twitter.get_news_feed(1));

        twitter.follow(2, 1);
        ref_twitter.follow(2, 1);
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));

        twitter.follow(2, 3);
        ref_twitter.follow(2, 3);
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));

        twitter.follow(2, 4);
        ref_twitter.follow(2, 4);
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));

        twitter.unfollow(2, 1);
        ref_twitter.unfollow(2, 1);
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));

        twitter.unfollow(2, 3);
        ref_twitter.unfollow(2, 3);
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));

        twitter.unfollow(2, 4);
        ref_twitter.unfollow(2, 4);
        assert_eq!(twitter.get_news_feed(2), ref_twitter.get_news_feed(2));

        twitter.follow(4, 3);
        ref_twitter.follow(4, 3);
        assert_eq!(twitter.get_news_feed(4), ref_twitter.get_news_feed(4));

        twitter.follow(3, 1);
        ref_twitter.follow(3, 1);
        assert_eq!(twitter.get_news_feed(3), ref_twitter.get_news_feed(3));

        twitter.follow(3, 4);
        ref_twitter.follow(3, 4);
        assert_eq!(twitter.get_news_feed(3), ref_twitter.get_news_feed(3));
        assert_eq!(twitter.get_news_feed(3), ref_twitter.get_news_feed(4));

        for user_id in 6..25 {
            twitter.post_tweet(user_id, user_id * 100);
            ref_twitter.post_tweet(user_id, user_id * 100);
            assert_eq!(
                twitter.get_news_feed(user_id),
                ref_twitter.get_news_feed(user_id)
            );

            twitter.post_tweet(user_id - 1, (user_id - 1) * 100 + 1);
            ref_twitter.post_tweet(user_id - 1, (user_id - 1) * 100 + 1);
            assert_eq!(
                twitter.get_news_feed(user_id - 1),
                ref_twitter.get_news_feed(user_id - 1)
            );

            twitter.follow(3, user_id);
            ref_twitter.follow(3, user_id);
            assert_eq!(twitter.get_news_feed(3), ref_twitter.get_news_feed(3));
        }

        // Uncomment to output leetcode test case input
        // eprintln!("[\"Twitter\",{}]", ref_twitter.method_calls.join(","));
        // eprintln!("[[],{}]", ref_twitter.method_params.join(","));
    }
}
