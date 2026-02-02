#include "exchange.h"
#include "http_client.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <thread>
#include <iostream>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Parse a JSON array of price levels into a vector of Level.
//
// Handles two formats transparently:
//   Coinbase: [["105000.00", "0.5"], ...]          <- each entry is an array
//   Gemini:   [{"price":"105000.00","amount":"0.5"}, ...]  <- each entry is an object

static std::vector<Level> parse_levels(const json& arr, const std::string& size_key) {
    std::vector<Level> out;
    for (size_t i = 0; i < arr.size(); ++i) {
        try {
            double price, size;
            if (arr[i].is_array()) {
                price = std::stod(arr[i][0].get<std::string>());
                size  = std::stod(arr[i][1].get<std::string>());
            } else {
                price = std::stod(arr[i]["price"].get<std::string>());
                size  = std::stod(arr[i][size_key].get<std::string>());
            }
            if (price > 0 && size > 0)
                out.push_back({price, size});
        } catch (...) {
            continue;
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// Exchange::fetch
// ---------------------------------------------------------------------------
Book Exchange::fetch() {
    // Yield in 50 ms slices until the rate limiter window opens.
    while (!limiter.try_acquire())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::string body = http_get(url);
    json root = json::parse(body);

    Book book;
    book.name = name;

    if (root.contains("bids"))
        book.bids = parse_levels(root["bids"], size_key);
    if (root.contains("asks"))
        book.asks = parse_levels(root["asks"], size_key);

    // Normalise: bids descending, asks ascending
    std::sort(book.bids.begin(), book.bids.end(),
              [](const Level& a, const Level& b){ return a.price > b.price; });
    std::sort(book.asks.begin(), book.asks.end(),
              [](const Level& a, const Level& b){ return a.price < b.price; });

    return book;
}

// ---------------------------------------------------------------------------
// Factory functions
// ---------------------------------------------------------------------------
Exchange make_coinbase() {
    return {
        "Coinbase",
        "https://api.exchange.coinbase.com/products/BTC-USD/book?level=2",
        RateLimiter(std::chrono::seconds(2)),
        "size"
    };
}

Exchange make_gemini() {
    return {
        "Gemini",
        "https://api.gemini.com/v1/book/BTCUSD",
        RateLimiter(std::chrono::seconds(2)),
        "amount"
    };
}
