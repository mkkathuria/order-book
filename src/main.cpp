#include <iostream>
#include <vector>
#include <future>
#include <algorithm>
#include <string>

#include "exchange.h"   
#include "order_book.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    // --- Parse --qty (default: 10 BTC) ---
    std::cout << "hello afd" << std::endl;
    double qty = 10.0;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--qty" && i + 1 < argc) {
            qty = std::stod(argv[++i]);
            if (qty <= 0) {
                std::cerr << "Error: --qty must be positive\n";
                return 1;
            }
        }
    }

    // --- Create exchanges ---
    Exchange coinbase = make_coinbase();
    Exchange gemini   = make_gemini();

    // // --- Fetch both concurrently ---
    auto fut_cb = std::async(std::launch::async, [&]{ return coinbase.fetch(); });
    auto fut_gm = std::async(std::launch::async, [&]{ return gemini.fetch();   });

    // // --- Collect results and merge into single bid/ask lists ---
    std::vector<Level> all_bids, all_asks;

    auto collect = [&](std::future<Book>& fut) {
        try {
            Book b = fut.get();
            std::cout << "[" << b.name << "] "
                      << b.bids.size() << " bids, "
                      << b.asks.size()  << " asks\n";
            all_bids.insert(all_bids.end(), b.bids.begin(), b.bids.end());
            all_asks.insert(all_asks.end(), b.asks.begin(), b.asks.end());
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    };

    collect(fut_cb);
    collect(fut_gm);

    // // Sort the merged book: bids descending, asks ascending
    std::sort(all_bids.begin(), all_bids.end(),
              [](const Level& a, const Level& b){ return a.price > b.price; });
    std::sort(all_asks.begin(), all_asks.end(),
              [](const Level& a, const Level& b){ return a.price < b.price; });

    std::cout << "Aggregated: " << all_bids.size() << " bids, "
                                 << all_asks.size() << " asks\n";

    // // --- Execute and print ---
    double buy_cost = execute(all_asks, qty);   // walk asks cheapest-first
    double sell_rev = execute(all_bids, qty);   // walk bids most-expensive-first

    std::cout << "To buy "  << qty << " BTC: " << format_usd(buy_cost) << "\n";
    std::cout << "To sell " << qty << " BTC: " << format_usd(sell_rev) << "\n";

    return 0;
}
