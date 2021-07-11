use std::collections::{BTreeMap, HashMap, HashSet};
use std::convert::TryInto;

struct Token {
    expiry_time: i32,
}

pub struct AuthenticationManager {
    current_time: i32,
    ttl: i32,
    tokens: HashMap<String, Token>,
    tokens_by_expiry: BTreeMap<i32, HashSet<String>>,
}

impl AuthenticationManager {
    pub fn new(time_to_live: i32) -> Self {
        assert!(1 <= time_to_live && time_to_live <= 100_000_000);
        Self {
            current_time: 1,
            ttl: time_to_live,
            tokens: Default::default(),
            tokens_by_expiry: Default::default(),
        }
    }

    fn remove_expired(&mut self) {
        // early exit if there are no tokens to expire
        if let Some((&time, _)) = self.tokens_by_expiry.iter().next() {
            if time > self.current_time {
                return;
            }
        }

        // split the btree into expired and unexpired tokens
        let unexpired_tokens = self.tokens_by_expiry.split_off(&(self.current_time + 1));
        // put the unexpired ones back into our tree and then remove all the expired ones
        let expired_tokens = std::mem::replace(&mut self.tokens_by_expiry, unexpired_tokens);
        for (_, token_strings) in expired_tokens {
            for token in token_strings {
                self.tokens.remove(&token);
            }
        }
    }

    fn update_time(&mut self, current_time: i32) {
        assert!(self.current_time <= current_time);
        self.current_time = current_time;
        self.remove_expired();
    }

    pub fn generate(&mut self, token_id: String, current_time: i32) {
        self.update_time(current_time);

        let expiry_time = self.current_time + self.ttl;
        let token = Token { expiry_time };

        self.tokens.insert(token_id.clone(), token);
        self.tokens_by_expiry
            .entry(expiry_time)
            .or_default()
            .insert(token_id);
    }

    pub fn renew(&mut self, token_id: String, current_time: i32) {
        self.update_time(current_time);

        if let Some(token) = self.tokens.get_mut(&token_id) {
            // remove/update/re-add to tokens by expiry
            self.tokens_by_expiry
                .get_mut(&token.expiry_time)
                .unwrap()
                .remove(&token_id);

            token.expiry_time = self.current_time + self.ttl;

            self.tokens_by_expiry
                .entry(token.expiry_time)
                .or_default()
                .insert(token_id);
        }
    }

    pub fn count_unexpired_tokens(&mut self, current_time: i32) -> i32 {
        self.update_time(current_time);
        self.tokens.len().try_into().unwrap()
    }
}

#[cfg(test)]
mod tests {
    use super::AuthenticationManager;

    #[test]
    fn example() {
        let mgr = &mut AuthenticationManager::new(5);

        mgr.renew("aaa".to_owned(), 1);
        mgr.generate("aaa".to_owned(), 2);
        assert_eq!(mgr.count_unexpired_tokens(6), 1);
        mgr.generate("bbb".to_owned(), 7);
        mgr.renew("aaa".to_owned(), 8);
        mgr.renew("bbb".to_owned(), 10);
        assert_eq!(mgr.count_unexpired_tokens(14), 1);
        assert_eq!(mgr.count_unexpired_tokens(15), 0);
    }

    #[test]
    fn example2() {
        let mgr = &mut AuthenticationManager::new(10);

        mgr.generate("aaa".to_owned(), 1);
        mgr.generate("bbb".to_owned(), 1);
        mgr.generate("ccc".to_owned(), 4);
        assert_eq!(mgr.count_unexpired_tokens(4), 3);
        mgr.renew("bbb".to_owned(), 4);
        assert_eq!(mgr.count_unexpired_tokens(11), 2);
        assert_eq!(mgr.count_unexpired_tokens(15), 0);

        mgr.generate("eee".to_owned(), 20);
        mgr.generate("fff".to_owned(), 20);
        mgr.generate("ggg".to_owned(), 20);

        assert_eq!(mgr.count_unexpired_tokens(31), 0);
    }
}
