#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include <gtest/gtest.h>
#include <boost/container_hash/hash.hpp>

std::regex patternToRegex(const std::string &pattern) {
    // convert pattern to an equivalent regex
    std::string regex_pat;
    regex_pat.reserve(pattern.size() * 2);
    // since pattern is made of only lowercase english letters, we only need
    // to take care of the special cases of '*' and '?'
    for (char c : pattern) {
        if (c == '*') {
            regex_pat += ".*";
        } else if (c == '?') {
            regex_pat += '.';
        } else {
            regex_pat += c;
        }
    }
    return std::regex(regex_pat);
}

bool isMatchRegex(const std::string &input, const std::string &pattern) {
    auto regex = patternToRegex(pattern);
    return std::regex_match(input, regex);
}

struct TupleHash {
    template<class ...Args>
    std::size_t operator()(const std::tuple<Args...> &value) const {
        return hash_impl(value, std::index_sequence_for<Args...>{});
    }

private:
    template<class ...Args, std::size_t ...Is>
    std::size_t hash_impl(const std::tuple<Args...> &value,
                          std::index_sequence<Is...>) const {
        std::size_t seed = 0;
        // adapted from boost::hash_combine
        auto hash_combine = [&seed](auto &x) {
            using T = std::remove_const_t<std::remove_reference_t<decltype(x)>>;
            auto hash = std::hash<T>()(x);
            seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        // hash all tuple values & combine
        (hash_combine(std::get<Is>(value)) , ...);
        return seed;
    }
};

template<class, class...>
class Memoizer;

template<class R, class ...Args>
class Memoizer<R(Args...)> {
public:
    std::optional<R> & get(Args ...args) {
        return cache_[std::tuple{args...}];
    }

    void reserve(std::size_t n) { cache_.reserve(n); }
    void clear() { cache_.clear(); }
private:
    std::unordered_map<std::tuple<Args...>, std::optional<R>, TupleHash> cache_;
};

class Solution {
public:
    static bool isMatch(const std::string &input, std::string pattern) {
        // start out by coalescing repeated wildcards and then finding the number
        // of non-wildcard characters in pattern so that we can sort-circuit
        // impossible matches
        auto new_pat = coalescePatternWildcards(std::move(pattern));
        auto num_wildcard = std::count_if(new_pat.begin(), new_pat.end(), [](char c) { return c == '*'; });
        auto needed_input_chars = new_pat.size() - num_wildcard;

        // build a memoizer, this puts an upper limit of O(n * m) on runtime
        static thread_local Memoizer<bool(std::size_t, std::size_t)> memoizer;
        memoizer.clear();
        memoizer.reserve((input.size() + 1) * (new_pat.size() + 1));

        return isMatchRecursive(input, new_pat, needed_input_chars, memoizer);
    }

private:
    static std::string coalescePatternWildcards(std::string pattern) {
        // coalesce runs of '*'
        char last = '\0';
        auto out = pattern.begin();

        for (char c : pattern) {
            if (c != '*' || last != '*') {
                *out++ = c;
            }
            last = c;
        }
        pattern.resize(std::distance(pattern.begin(), out));
        return std::move(pattern);
    }

    static bool isMatchRecursive(const std::string_view input,
                                 const std::string_view pattern,
                                 std::size_t needed_input_chars,
                                 Memoizer<bool(std::size_t, std::size_t)> &memoizer)
    {
        if (input.size() < needed_input_chars) {
            // short-circuit if the input string doesn't have enough characters
            // to fulfill the non-'*' elements of pattern
            return false;
        } else if (pattern.empty()) {
            return input.empty();
        } else if (input.empty()) {
            // since we coalesce repeated wildcards, the only way we
            // can match an empty string is is pattern contains a single wildcard
            return pattern == "*";
        }

        // try to get the memoized result
        auto &memoized_result = memoizer.get(input.size(), pattern.size());
        if (memoized_result) {
            return *memoized_result;
        }

        // neither are empty
        auto next_char = input.front();
        auto next_pattern = pattern.front();

        switch (next_pattern) {
            case '*':
                // prefer greedy matching and try to consume as much as we can, but
                // fall back to treating '*' as empty if that fails
                memoized_result =
                    isMatchRecursive(input.substr(1), pattern, needed_input_chars, memoizer) ||
                    isMatchRecursive(input, pattern.substr(1), needed_input_chars, memoizer);
                return *memoized_result;
            case '?':
                // consume one input char and one pattern char
                memoized_result =
                        isMatchRecursive(input.substr(1), pattern.substr(1), needed_input_chars - 1, memoizer);
                return *memoized_result;
            default:
                // consume one input char and one pattern char, but short-circuit if the current ones don't match
                memoized_result =
                        next_char == next_pattern &&
                        isMatchRecursive(input.substr(1), pattern.substr(1), needed_input_chars - 1, memoizer);
                return *memoized_result;
        }
    }
};

void checkOne(const std::string &input, const std::string &pattern) {
    bool expected = isMatchRegex(input, pattern);
    bool actual = Solution::isMatch(input, pattern);
    EXPECT_EQ(expected, actual)
        << "  input:   " << input << '\n'
        << "  pattern: " << pattern;
}

void checkCase(const std::string &input, const std::string &pattern) {
    checkOne(input, pattern);
    checkOne(input + "a", pattern + "a");
    checkOne(input + "z", pattern + "?");
    checkOne(input + "z", pattern);
    checkOne("z" + input, pattern);
    checkOne(input, pattern + "*");
    checkOne(input, "*" + pattern);
    checkOne(input, pattern + "?");
    checkOne(input, "?" + pattern);
    checkOne(input + "z", pattern + "?");
    checkOne("z" + input, "?" + pattern);
    checkOne(input + "abcd", pattern + "*");
    checkOne("abcd" + input, "*" + pattern);
}

TEST(Solution, WildcardMatching) {
    checkCase("", "?");
    checkCase("", "*");
    checkCase("", "");
    checkCase("a", "?");
    checkCase("a", "*");
    checkCase("a", "a");
    checkCase("a", "z");
    checkCase("aa", "a");
    checkCase("aa", "*");
    checkCase("cb", "?a");
    checkCase("adceb", "*a*b");
    checkCase("acdcb", "a*c?b");
    checkCase("acdcbacdcbacdcbacdcbacdcb",
              "a*c?ba*c?ba*c?ba*c?ba*c?b");
    checkCase("", "***********");
    checkCase("a", "***********");

    std::string pathological_str = "abcdefghijklmnopqrstuvwxyz"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "abcdefghijklmnopqrstuvwxyz";
    std::string pathological_pat = "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z*"
                                   "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z*"
                                   "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z*"
                                   "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z*";
    checkCase(pathological_str, pathological_pat);

    std::string pathological_str2;
    std::string pathological_pat2;
    for (int i = 0; i < 1000; ++i) {
        char next = static_cast<char>('a' + (i % 26));
        pathological_str2 += next;
        pathological_pat2 += next;
        pathological_pat2 += '*';
    }
    EXPECT_EQ(Solution::isMatch(pathological_str2, pathological_pat2), true);
    pathological_str2 += "x";
    EXPECT_EQ(Solution::isMatch(pathological_str2, pathological_pat2), true);
    pathological_str2 += "x";
    pathological_pat2 += "?";
    EXPECT_EQ(Solution::isMatch(pathological_str2, pathological_pat2), true);
    pathological_str2 += "a";
    pathological_pat2 += "b";
    EXPECT_EQ(Solution::isMatch(pathological_str2, pathological_pat2), false);

    std::string pathological_str3(1000, 'a');
    pathological_str3[0] = 'z';
    std::string pathological_pat3(1000, '*');
    pathological_pat3[pathological_pat3.size() - 2] = 'z';
    EXPECT_EQ(Solution::isMatch(pathological_str3, pathological_pat3), true);

    std::string pathological_str4 = pathological_str3;
    std::string pathological_pat4;
    for (std::size_t i = 0; i < pathological_str4.size(); ++i) {
        pathological_pat4 += "*?";
    }
    pathological_pat4[1] = 'z';
    EXPECT_EQ(Solution::isMatch(pathological_str4, pathological_pat4), true);
    EXPECT_EQ(Solution::isMatch("abbbabaaabbabbabbabaabbbaabaaaabbbabaaabbbbbaaababbb", "*a*b*aa*b*bbb*ba*a"), false);
    EXPECT_EQ(Solution::isMatch("abbbabaaabbabbabbabaabbbaabaaaabbbabaaabbbbbaaababbb", "**a*b*aa***b***bbb*ba*a"), false);
    EXPECT_EQ(Solution::isMatch("abbbabaaabbabbabbabaabbbaabaaaabbbabaaabbbbbaaababbbabbbabaaabbabbabbabaabbbaabaaaabbbabaaabbbbbaaababbb",
                                "*a*b*aa*b*bbb*ba*a*a*b*aa*b*bbb*ba*a"), false);
}
