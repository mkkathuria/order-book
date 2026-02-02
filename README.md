# BTC-USD Order Book Aggregator

Fetches live order books from Coinbase and Gemini, merges them, and calculates the best-execution price to buy or sell Bitcoin.

--

```bash
# One-liner (no CMake needed):
g++ -std=c++17 -O2 -I src -I third_party \
    src/main.cpp src/rate_limiter.cpp src/http_client.cpp \
    src/exchange.cpp src/order_book.cpp \
    -lpthread -o build/orderbook

```

## Usage

```bash
./orderbook --qty 5.5  # custom quantity
```

### Example output

```
[Coinbase] 250 bids, 250 asks
[Gemini] 300 bids, 300 asks
Aggregated: 550 bids, 550 asks
To buy 10 BTC: $1,050,234.56
To sell 10 BTC: $1,049,876.12
```

---
