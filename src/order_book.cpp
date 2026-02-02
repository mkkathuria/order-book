#include "order_book.h"
#include <algorithm>
#include <iostream>

double execute(const std::vector<Level>& levels, double qty) {
    double remaining = qty;
    double total     = 0.0;

    for (const auto& lv : levels) {
        if (remaining <= 0) break;
        double fill = std::min(lv.size, remaining);
        total     += fill * lv.price;
        remaining -= fill;
    }

    if (remaining > 1e-9)
        std::cerr << "Warning: only filled " << (qty - remaining)
                  << " of " << qty << " BTC (insufficient liquidity).\n";

    return total;
}
