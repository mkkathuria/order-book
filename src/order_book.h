#pragma once

#include <vector>
#include "exchange.h"  // for Level

// Walk a sorted list of price levels and greedily fill `qty` BTC.
//
// For buying:  pass asks sorted ascending (cheapest first).
// For selling: pass bids sorted descending (most expensive first).
//
// Returns the total USD cost/revenue for the filled portion.
// Prints a warning to stderr if liquidity is insufficient to fill the full qty.
double execute(const std::vector<Level>& levels, double qty);
