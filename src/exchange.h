#pragma once

#include <string>
#include <vector>
#include "rate_limiter.h"

struct Level {
    double price;
    double size;
};

struct Book {
    std::string          name;
    std::vector<Level>   bids;  // sorted descending by price
    std::vector<Level>   asks;  // sorted ascending by price
};

// Represents one exchange: where to fetch, how to parse, and rate limiting.
struct Exchange {
    std::string  name;
    std::string  url;
    RateLimiter  limiter;
    std::string  size_key;  // "size" for Coinbase, "amount" for Gemini

    // Fetch the order book from this exchange.
    // Blocks (via short yields) until the rate limiter allows the call,
    // then fetches, parses, and returns the sorted book.
    Book fetch();
};

// Pre-built exchange instances ready to use.
Exchange make_coinbase();
Exchange make_gemini();
