#pragma once

#include <chrono>
#include <mutex>

// Non-blocking rate limiter. One instance per exchange.
// try_acquire() returns true only if the minimum interval has passed
// since the last successful acquire. It never sleeps or blocks internally —
// the caller is responsible for yielding until it returns true.
class RateLimiter {
public:
    explicit RateLimiter(std::chrono::steady_clock::duration interval);

    bool try_acquire();

private:
    std::chrono::steady_clock::duration   interval_;
    std::chrono::steady_clock::time_point last_;
    bool   called_ = false;
    std::mutex mu_;
};
